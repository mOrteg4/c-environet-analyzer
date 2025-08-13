#!/bin/bash
# Script to install all required dependencies for the EnviroNet Analyzer project

echo "Installing EnviroNet Analyzer dependencies..."

# Update package lists
sudo apt update

# Install Python and development tools
sudo apt install -y python3 python3-pip python3-venv build-essential

# Install network tools
sudo apt install -y iw wireless-tools net-tools iperf3 

# Install GPIO libraries (for real hardware mode)
sudo apt install -y python3-rpi.gpio

# Create virtual environment and install Python dependencies
python3 -m venv .venv
source .venv/bin/activate
pip install gpiozero RPi.GPIO pyyaml matplotlib pandas

echo "Dependencies installed successfully!"
echo "Activate the virtual environment with: source .venv/bin/activate"