#!/bin/bash

# Configuración
WAN_IFACE="wlo1"
LAN_IFACE="eno1"
LAN_IP="192.168.50.1"
LAN_NETMASK="24"

# Comprobar root
if [ "$EUID" -ne 0 ]; then
  echo "Por favor, ejecuta como root (sudo)"
  exit 1
fi

echo "--- Configurando PC como Gateway ---"
echo "WAN: $WAN_IFACE"
echo "LAN: $LAN_IFACE"

# 1. Configurar IP en la interfaz LAN
echo "[*] Asignando IP $LAN_IP a $LAN_IFACE..."
ip addr flush dev $LAN_IFACE
ip addr add $LAN_IP/$LAN_NETMASK dev $LAN_IFACE
ip link set $LAN_IFACE up

# 2. Habilitar Forwarding
echo "[*] Habilitando IP Forwarding..."
sysctl -w net.ipv4.ip_forward=1 > /dev/null

# 3. Configurar NAT (IPTables)
echo "[*] Configurando NAT..."
# Limpiar reglas anteriores (opcional, comentado para seguridad)
# iptables -F
# iptables -t nat -F

# Enmascarar tráfico saliente por la WAN
iptables -t nat -A POSTROUTING -o $WAN_IFACE -j MASQUERADE

# Permitir tráfico entre interfaces
iptables -A FORWARD -i $WAN_IFACE -o $LAN_IFACE -m state --state RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i $LAN_IFACE -o $WAN_IFACE -j ACCEPT

echo "[OK] Configuración de red completada."
echo "Ahora puedes ejecutar el servidor DHCP (script python) o configurar la IP estática en el ESP32."
