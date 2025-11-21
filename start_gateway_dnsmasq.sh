#!/bin/bash

# Configuración
WAN_IFACE="wlo1"
LAN_IFACE="eno1"
LAN_IP="192.168.50.1"
NETMASK="255.255.255.0"
DHCP_RANGE="192.168.50.50,192.168.50.150,12h"

# Comprobar root
if [ "$EUID" -ne 0 ]; then
  echo "Este script necesita permisos de superusuario."
  echo "Ejecutando con sudo..."
  exec sudo -S "$0" "$@" <<< "2023"
fi

echo "--- Iniciando Gateway con DNSMASQ ---"

# 1. Limpiar configuraciones previas
echo "[*] Deteniendo servicios conflictivos..."
systemctl stop dnsmasq
killall dnsmasq 2>/dev/null
killall python3 2>/dev/null

# 2. Configurar IP en LAN
echo "[*] Configurando IP $LAN_IP en $LAN_IFACE..."
ip addr flush dev $LAN_IFACE
ip addr add $LAN_IP/24 dev $LAN_IFACE
ip link set $LAN_IFACE up

# 3. Configurar NAT e IP Forwarding
echo "[*] Configurando NAT y Forwarding..."
sysctl -w net.ipv4.ip_forward=1 > /dev/null
iptables -t nat -F
iptables -F
iptables -t nat -A POSTROUTING -o $WAN_IFACE -j MASQUERADE
iptables -A FORWARD -i $WAN_IFACE -o $LAN_IFACE -m state --state RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i $LAN_IFACE -o $WAN_IFACE -j ACCEPT

# 4. Crear archivo de configuración temporal para dnsmasq
CONF_FILE="/etc/dnsmasq_esp32.conf"
echo "[*] Generando configuración de dnsmasq en $CONF_FILE..."
rm -f $CONF_FILE
cat > $CONF_FILE <<EOF
interface=$LAN_IFACE
bind-interfaces
dhcp-range=$DHCP_RANGE
dhcp-option=3,$LAN_IP
dhcp-option=6,8.8.8.8,8.8.4.4
log-queries
log-dhcp
EOF

# 5. Iniciar dnsmasq
echo "[*] Iniciando dnsmasq (Daemon)..."
echo "2023" | sudo -S dnsmasq -C $CONF_FILE --log-facility="/home/andoni/Escritorio/esp nuttx/dnsmasq_debug.log"

