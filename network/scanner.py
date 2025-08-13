import subprocess
import re
import json
import logging
import time
from typing import List, Dict, Any, Optional
from dataclasses import dataclass

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    filename='network_scanner.log'
)
logger = logging.getLogger('network.scanner')

@dataclass
class WiFiNetwork:
    """Represents a WiFi network detected during scanning."""
    ssid: str
    bssid: str
    signal_strength: int  # in dBm
    channel: int
    frequency: str
    encryption: str
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for serialization."""
        return {
            'ssid': self.ssid,
            'bssid': self.bssid,
            'signal_strength': self.signal_strength,
            'channel': self.channel,
            'frequency': self.frequency,
            'encryption': self.encryption,
            'timestamp': time.time()
        }

class NetworkScanner:
    """Scanner for WiFi networks using the PandaWireless adapter."""
    
    def __init__(self, interface: str = 'wlan0'):
        self.interface = interface
        logger.info(f"Initialized NetworkScanner on interface {interface}")
    
    def scan_networks(self) -> List[WiFiNetwork]:
        """Scan for available WiFi networks."""
        try:
            logger.info(f"Starting network scan on {self.interface}")
            # Run the iwlist command to scan for networks
            result = subprocess.run(
                ['sudo', 'iwlist', self.interface, 'scan'],
                capture_output=True, 
                text=True,
                check=True
            )
            
            # Parse the output
            networks = self._parse_scan_output(result.stdout)
            logger.info(f"Scan complete. Found {len(networks)} networks")
            return networks
            
        except subprocess.SubprocessError as e:
            logger.error(f"Error scanning networks: {e}")
            return []
    
    def _parse_scan_output(self, output: str) -> List[WiFiNetwork]:
        """Parse the output of iwlist scan."""
        networks = []
        current_network = {}
        
        for line in output.split('\n'):
            line = line.strip()
            
            # New cell means new network
            if line.startswith('Cell '):
                if current_network and 'ssid' in current_network:
                    # Add previous network to list
                    networks.append(WiFiNetwork(
                        ssid=current_network.get('ssid', 'Unknown'),
                        bssid=current_network.get('bssid', '00:00:00:00:00:00'),
                        signal_strength=current_network.get('signal_strength', 0),
                        channel=current_network.get('channel', 0),
                        frequency=current_network.get('frequency', ''),
                        encryption=current_network.get('encryption', 'unknown')
                    ))
                # Start new network
                current_network = {}
                bssid_match = re.search(r'Address: ([0-9A-F:]{17})', line)
                if bssid_match:
                    current_network['bssid'] = bssid_match.group(1)
            
            # Extract ESSID (network name)
            elif 'ESSID:' in line:
                ssid_match = re.search(r'ESSID:"([^"]*)"', line)
                if ssid_match:
                    current_network['ssid'] = ssid_match.group(1)
            
            # Extract signal level
            elif 'Signal level=' in line:
                signal_match = re.search(r'Signal level=(-\d+) dBm', line)
                if signal_match:
                    current_network['signal_strength'] = int(signal_match.group(1))
            
            # Extract channel/frequency
            elif 'Frequency:' in line:
                freq_match = re.search(r'Frequency:([0-9.]+) GHz', line)
                channel_match = re.search(r'\(Channel (\d+)\)', line)
                if freq_match:
                    current_network['frequency'] = freq_match.group(1) + ' GHz'
                if channel_match:
                    current_network['channel'] = int(channel_match.group(1))
            
            # Extract encryption
            elif 'Encryption key:' in line:
                if 'on' in line:
                    current_network['encryption'] = 'encrypted'
                else:
                    current_network['encryption'] = 'open'
        
        # Add the last network if it exists
        if current_network and 'ssid' in current_network:
            networks.append(WiFiNetwork(
                ssid=current_network.get('ssid', 'Unknown'),
                bssid=current_network.get('bssid', '00:00:00:00:00:00'),
                signal_strength=current_network.get('signal_strength', 0),
                channel=current_network.get('channel', 0),
                frequency=current_network.get('frequency', ''),
                encryption=current_network.get('encryption', 'unknown')
            ))
        
        return networks
    
    def save_scan_results(self, filename: str) -> None:
        """Scan networks and save results to a JSON file."""
        networks = self.scan_networks()
        data = {
            'timestamp': time.time(),
            'interface': self.interface,
            'networks': [n.to_dict() for n in networks]
        }
        
        try:
            with open(filename, 'w') as f:
                json.dump(data, f, indent=2)
            logger.info(f"Saved scan results to {filename}")
        except IOError as e:
            logger.error(f"Error saving scan results: {e}")

    def get_connected_network_info(self) -> Optional[Dict[str, Any]]:
        """Get information about the currently connected network."""
        try:
            result = subprocess.run(
                ['iwconfig', self.interface],
                capture_output=True,
                text=True,
                check=True
            )
            
            output = result.stdout
            info = {}
            
            # Extract ESSID
            essid_match = re.search(r'ESSID:"([^"]*)"', output)
            if essid_match:
                info['ssid'] = essid_match.group(1)
            else:
                # Not connected
                return None
                
            # Extract signal quality
            quality_match = re.search(r'Signal level=(-\d+) dBm', output)
            if quality_match:
                info['signal_strength'] = int(quality_match.group(1))
            
            # Get bit rate
            bitrate_match = re.search(r'Bit Rate=([0-9.]+ [GM]b/s)', output)
            if bitrate_match:
                info['bit_rate'] = bitrate_match.group(1)
                
            return info
            
        except subprocess.SubprocessError as e:
            logger.error(f"Error getting connected network info: {e}")
            return None