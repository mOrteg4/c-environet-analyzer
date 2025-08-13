#!/bin/bash
# Script to set up PandaWireless adapter as primary network interface

# Log file
LOG_FILE="/var/log/panda_setup.log"

log() {
    echo "$(date +"%Y-%m-%d %H:%M:%S") - $1" | tee -a $LOG_FILE
}

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root"
    exit 1
fi

log "Starting PandaWireless adapter setup"

# Check if adapter is connected
if ! lsusb | grep -q "Ralink" && ! lsusb | grep -q "Realtek"; then
    log "ERROR: PandaWireless adapter not detected. Please connect it and try again."
    exit 1
fi

# Find the adapter interface name
WIFI_INTERFACE=$(iw dev | grep Interface | awk '{print $2}' | grep -v "wlan0" | head -1)

if [ -z "$WIFI_INTERFACE" ]; then
    # If no other interface found, assume wlan0 is the PandaWireless
    WIFI_INTERFACE="wlan0"
    log "Using default interface name: $WIFI_INTERFACE"
else
    log "Found PandaWireless interface: $WIFI_INTERFACE"
fi

# Create network configuration for the adapter
cat > /etc/netplan/99-panda-wireless.yaml << EOF
network:
  version: 2
  renderer: networkd
  wifis:
    $WIFI_INTERFACE:
      dhcp4: true
      optional: true
      access-points:
        "YourWiFiSSID":
          password: "YourWiFiPassword"
EOF

log "Created netplan configuration for $WIFI_INTERFACE"

# Apply the configuration
log "Applying netplan configuration..."
netplan apply

# Verify the adapter is working
if ip addr show $WIFI_INTERFACE | grep -q "inet "; then
    log "SUCCESS: PandaWireless adapter configured and connected"
else
    log "PandaWireless adapter configured but not connected yet. Check WiFi credentials."
fi

log "Setup complete."