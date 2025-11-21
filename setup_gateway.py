import socket
import struct
import os
import subprocess
import sys
import time
import threading

# --- Configuration ---
# IP for the PC's Ethernet Interface
SERVER_IP = "192.168.50.1"
# IP to assign to the ESP32
CLIENT_IP = "192.168.50.2"
NETMASK = "255.255.255.0"
ROUTER = SERVER_IP
DNS = "8.8.8.8"
LEASE_TIME = 86400

def get_mac(bytes_data):
    return ":".join(map(lambda b: format(b, "02x"), bytes_data))

def ip_to_bytes(ip):
    return socket.inet_aton(ip)

def build_packet(xid, chaddr, message_type, client_ip=None):
    # DHCP Packet Structure
    # OP (1), HTYPE (1), HLEN (1), HOPS (1), XID (4), SECS (2), FLAGS (2), 
    # CIADDR (4), YIADDR (4), SIADDR (4), GIADDR (4), CHADDR (16), SNAME (64), FILE (128)
    # OPTIONS (Variable)
    
    op = 2 # BOOTREPLY
    htype = 1 # Ethernet
    hlen = 6
    hops = 0
    
    ciaddr = b'\x00\x00\x00\x00'
    yiaddr = ip_to_bytes(CLIENT_IP) # Your (Client) IP
    siaddr = ip_to_bytes(SERVER_IP) # Server IP
    giaddr = b'\x00\x00\x00\x00'
    
    # Magic Cookie
    magic_cookie = b'\x63\x82\x53\x63'
    
    # Options
    options = b''
    options += b'\x35\x01' + bytes([message_type]) # Option 53: Message Type
    options += b'\x01\x04' + ip_to_bytes(NETMASK)  # Option 1: Subnet Mask
    options += b'\x03\x04' + ip_to_bytes(ROUTER)   # Option 3: Router
    options += b'\x06\x04' + ip_to_bytes(DNS)      # Option 6: DNS Server
    options += b'\x33\x04' + struct.pack('!I', LEASE_TIME) # Option 51: Lease Time
    options += b'\x36\x04' + ip_to_bytes(SERVER_IP) # Option 54: DHCP Server Identifier
    options += b'\xff' # End Option

    packet = struct.pack('!BBBBIHH4s4s4s4s16s64s128s', 
                         op, htype, hlen, hops, xid, 0, 0,
                         ciaddr, yiaddr, siaddr, giaddr, chaddr, b'\x00'*64, b'\x00'*128)
    
    return packet + magic_cookie + options

def run_dhcp_server(interface):
    print(f"[*] Starting Python DHCP Server on {interface} ({SERVER_IP})...")
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    try:
        sock.bind((SERVER_IP, 67))
    except Exception as e:
        print(f"[!] Error binding to port 67: {e}")
        print("    Make sure you run this script with sudo and no other DHCP server is running (like dnsmasq).")
        return

    while True:
        try:
            data, addr = sock.recvfrom(1024)
            if len(data) < 240: continue
            
            # Parse minimal header
            op, htype, hlen, hops, xid, secs, flags, ciaddr, yiaddr, siaddr, giaddr, chaddr = struct.unpack('!BBBBIHH4s4s4s4s16s', data[:44])
            
            # Check for Magic Cookie
            if data[236:240] != b'\x63\x82\x53\x63': continue
            
            # Parse Options to find Message Type
            msg_type = 0
            opts = data[240:]
            i = 0
            while i < len(opts):
                opt_code = opts[i]
                if opt_code == 255: break # End
                if opt_code == 0: 
                    i += 1
                    continue
                opt_len = opts[i+1]
                if opt_code == 53: # DHCP Message Type
                    msg_type = opts[i+2]
                i += 2 + opt_len

            mac_addr = get_mac(chaddr[:6])
            
            if msg_type == 1: # DISCOVER
                print(f"[*] Received DHCP DISCOVER from {mac_addr}")
                packet = build_packet(xid, chaddr, 2) # OFFER
                sock.sendto(packet, ('255.255.255.255', 68))
                print(f"[*] Sent DHCP OFFER to {mac_addr} (IP: {CLIENT_IP})")
                
            elif msg_type == 3: # REQUEST
                print(f"[*] Received DHCP REQUEST from {mac_addr}")
                packet = build_packet(xid, chaddr, 5) # ACK
                sock.sendto(packet, ('255.255.255.255', 68))
                print(f"[*] Sent DHCP ACK to {mac_addr}")
                
        except Exception as e:
            print(f"[!] Error handling packet: {e}")

def setup_network(wan_iface, lan_iface):
    print(f"[*] Configuring Network...")
    print(f"    WAN (Internet): {wan_iface}")
    print(f"    LAN (ESP32):    {lan_iface}")
    
    # 1. Configure LAN IP
    print(f"[*] Setting IP {SERVER_IP} on {lan_iface}...")
    subprocess.run(f"ip addr flush dev {lan_iface}", shell=True)
    subprocess.run(f"ip addr add {SERVER_IP}/24 dev {lan_iface}", shell=True)
    subprocess.run(f"ip link set {lan_iface} up", shell=True)
    
    # 2. Enable Forwarding
    print("[*] Enabling IP Forwarding...")
    subprocess.run("sysctl -w net.ipv4.ip_forward=1", shell=True)
    
    # 3. Configure NAT (IPTables)
    print("[*] Configuring IPTables NAT...")
    # Flush existing rules for clean state (Optional, be careful)
    # subprocess.run("iptables -F", shell=True) 
    # subprocess.run("iptables -t nat -F", shell=True)
    
    subprocess.run(f"iptables -t nat -A POSTROUTING -o {wan_iface} -j MASQUERADE", shell=True)
    subprocess.run(f"iptables -A FORWARD -i {wan_iface} -o {lan_iface} -m state --state RELATED,ESTABLISHED -j ACCEPT", shell=True)
    subprocess.run(f"iptables -A FORWARD -i {lan_iface} -o {wan_iface} -j ACCEPT", shell=True)

if __name__ == "__main__":
    if os.geteuid() != 0:
        print("This script must be run as root (sudo).")
        sys.exit(1)

    print("--- ESP32 NuttX Gateway Setup ---")
    interfaces = os.listdir('/sys/class/net')
    print(f"Available Interfaces: {interfaces}")
    
    if len(sys.argv) < 3:
        print("Usage: sudo python3 setup_gateway.py <WAN_INTERFACE> <LAN_INTERFACE>")
        print("Example: sudo python3 setup_gateway.py wlan0 eth0")
        sys.exit(1)
        
    wan_iface = sys.argv[1]
    lan_iface = sys.argv[2]
    
    setup_network(wan_iface, lan_iface)
    run_dhcp_server(lan_iface)
