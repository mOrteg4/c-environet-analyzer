#!/bin/bash
#
# ==============================================================================
# Raspberry Pi Router Configuration Notes
# ==============================================================================
#
# Author: Jules
# Date: 2025-08-13
#
# --- DISCLAIMER ---
# This is NOT an executable script. It is a documentation file that outlines
# the steps and commands needed to configure a Raspberry Pi as a wireless
# access point (AP) and router. You should run these commands manually or
# adapt them into your own setup script.
#
# --- HARDWARE ASSUMPTIONS ---
# - Raspberry Pi 4 Model B
# - An external USB Wi-Fi adapter (e.g., Panda Wireless AC1200) that supports AP mode.
#   We will assume this adapter appears as 'wlan1'. The built-in Wi-Fi will be 'wlan0'.
# - The Pi is connected to the internet via its Ethernet port ('eth0').
#
# --- GOAL ---
# The Pi will take an internet connection from eth0 and create a new, firewalled
# Wi-Fi network on wlan1 for other devices to connect to.

# ==============================================================================
# STEP 1: SYSTEM UPDATE AND PACKAGE INSTALLATION
# ==============================================================================
# It's always a good practice to start with an up-to-date system.
# The core software we need is `hostapd` to create the access point and
# `dnsmasq` to handle DHCP (assigning IP addresses) and DNS.
# `netfilter-persistent` and `iptables-persistent` are used to save firewall rules.

# sudo apt update && sudo apt upgrade -y
# sudo apt install -y hostapd dnsmasq netfilter-persistent iptables-persistent


# ==============================================================================
# STEP 2: CONFIGURE THE DHCP SERVER (dnsmasq)
# ==============================================================================
# We need to configure dnsmasq to assign IP addresses to clients that connect
# to our new Wi-Fi network.
#
# First, back up the original config file:
# sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
#
# Then, create a new one. This configuration tells dnsmasq:
# - To listen for requests on the wlan1 interface.
# - To provide IP addresses in the range 192.168.4.20 to 192.168.4.50.
# - The lease time for these IPs is 24 hours.
# - The router's own IP is 192.168.4.1.

# sudo nano /etc/dnsmasq.conf
# --- (copy the following into the file) ---
# interface=wlan1
# dhcp-range=192.168.4.20,192.168.4.50,255.255.255.0,24h
# domain=wlan
# address=/gw.wlan/192.168.4.1


# ==============================================================================
# STEP 3: CONFIGURE THE ACCESS POINT (hostapd)
# ==============================================================================
# This is where we define our Wi-Fi network's name (SSID) and password.
#
# Create the configuration file:
# sudo nano /etc/hostapd/hostapd.conf
# --- (copy the following into the file) ---
# interface=wlan1
# driver=nl80211
# ssid=MyPiRouter
# hw_mode=g
# channel=7
# wmm_enabled=0
# macaddr_acl=0
# auth_algs=1
# ignore_broadcast_ssid=0
# wpa=2
# wpa_passphrase=MySecurePassword
# wpa_key_mgmt=WPA-PSK
# wpa_pairwise=TKIP
# rsn_pairwise=CCMP
#
# After creating the file, we need to tell the system where to find it:
# sudo nano /etc/default/hostapd
# --- (find the line #DAEMON_CONF and change it to) ---
# DAEMON_CONF="/etc/hostapd/hostapd.conf"


# ==============================================================================
# STEP 4: CONFIGURE NETWORK ADDRESS TRANSLATION (NAT)
# ==============================================================================
# This is the "router" part. We need to forward traffic from our Wi-Fi clients
# (on wlan1) to the internet (through eth0).
#
# First, enable IP forwarding:
# sudo nano /etc/sysctl.conf
# --- (uncomment the line) ---
# net.ipv4.ip_forward=1
#
# Now, add a firewall rule to handle the forwarding (NAT).
# sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
#
# To make this rule persist after a reboot, we save it.
# sudo netfilter-persistent save


# ==============================================================================
# STEP 5: ASSIGN A STATIC IP TO THE WI-FI INTERFACE
# ==============================================================================
# The router itself needs a fixed IP address on the new network.
#
# sudo nano /etc/network/interfaces.d/wlan1
# --- (add the following to the file) ---
# allow-hotplug wlan1
# iface wlan1 inet static
#     address 192.168.4.1
#     netmask 255.255.255.0
#     network 192.168.4.0
#     broadcast 192.168.4.255


# ==============================================================================
# STEP 6: REBOOT AND VERIFY
# ==============================================================================
# A reboot is the easiest way to ensure all services start in the correct order.
# sudo systemctl reboot
#
# After rebooting, you should see a new Wi-Fi network named "MyPiRouter".
# Connect to it with the password "MySecurePassword".
# You should have internet access.
#
# You can check the status of the services with:
# sudo systemctl status hostapd
# sudo systemctl status dnsmasq
#
# Good luck!
