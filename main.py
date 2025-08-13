#!/usr/bin/env python3
"""
EnviroNet Analyzer - Main integration script

Integrates sensor data with network performance metrics to analyze
environmental impacts on WiFi performance.
"""

import os
import time
import json
import logging
import argparse
from typing import Dict, Any, List
import threading
import csv
from datetime import datetime

# Import our modules
from sensors.interface import SensorFactory
from network.scanner import NetworkScanner
from network.performance import NetworkPerformanceTester

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    filename='environet.log'
)
logger = logging.getLogger('environet')

# Load configuration
def load_config(config_path: str = 'config.json') -> Dict[str, Any]:
    """Load configuration from JSON file."""
    try:
        if os.path.exists(config_path):
            with open(config_path, 'r') as f:
                return json.load(f)
        else:
            logger.warning(f"Config file {config_path} not found. Using defaults.")
            return {
                'interface': 'wlan0',
                'scan_interval': 60,
                'ultrasonic_trigger_pin': 23,
                'ultrasonic_echo_pin': 24,
                'ir_pin': 17,
                'log_directory': 'logs',
                'iperf_server': None,  # Optional iperf server
                'targets': ['8.8.8.8', 'google.com']
            }
    except Exception as e:
        logger.error(f"Error loading config: {e}")
        return {}

class EnviroNetAnalyzer:
    """Main class integrating sensors and network analysis."""
    
    def __init__(self, config_path: str = 'config.json'):
        """Initialize with configuration."""
        self.config = load_config(config_path)
        logger.info("Initializing EnviroNetAnalyzer")
        
        # Create log directory if it doesn't exist
        os.makedirs(self.config.get('log_directory', 'logs'), exist_ok=True)
        
        # Initialize network components
        self.network_scanner = NetworkScanner(self.config.get('interface', 'wlan0'))
        self.performance_tester = NetworkPerformanceTester(self.config.get('interface', 'wlan0'))
        
        # Initialize sensors
        self.ultrasonic_sensor = SensorFactory.create_ultrasonic_sensor(
            trigger_pin=self.config.get('ultrasonic_trigger_pin', 23),
            echo_pin=self.config.get('ultrasonic_echo_pin', 24)
        )
        
        self.ir_sensor = SensorFactory.create_ir_sensor(
            pin=self.config.get('ir_pin', 17)
        )
        
        # Initialize data storage
        self.csv_file = os.path.join(
            self.config.get('log_directory', 'logs'),
            f"environet_data_{time.strftime('%Y%m%d-%H%M%S')}.csv"
        )
        self._init_csv()
        
        # Flag to control continuous monitoring
        self.running = False
        
    def _init_csv(self):
        """Initialize CSV file with headers."""
        with open(self.csv_file, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow([
                'timestamp',
                'distance_cm',
                'motion_detected',
                'wifi_signal_strength',
                'connected_ssid',
                'ping_avg_ms',
                'packet_loss_pct',
                'download_mbps',
                'upload_mbps'
            ])
    
    def collect_single_datapoint(self) -> Dict[str, Any]:
        """Collect a single integrated datapoint from all sources."""
        data = {
            'timestamp': datetime.now().isoformat(),
            'sensors': {},
            'network': {}
        }
        
        # Collect sensor data
        try:
            data['sensors']['distance_cm'] = self.ultrasonic_sensor.distance()
            data['sensors']['motion_detected'] = self.ir_sensor.is_active()
        except Exception as e:
            logger.error(f"Error collecting sensor data: {e}")
            data['sensors']['error'] = str(e)
        
        # Collect network data
        try:
            # Get info about connected network
            connected_info = self.network_scanner.get_connected_network_info()
            if connected_info:
                data['network']['connected'] = connected_info
            
            # Ping test to first target
            if self.config.get('targets'):
                ping_result = self.performance_tester.ping_test(
                    self.config['targets'][0], count=5
                )
                data['network']['ping'] = ping_result
                
            # iperf test if server configured
            if self.config.get('iperf_server'):
                # Just do download test for regular data points (faster)
                iperf_result = self.performance_tester.iperf_test(
                    self.config['iperf_server'],
                    duration=5,
                    direction='download'
                )
                data['network']['iperf'] = iperf_result
                
        except Exception as e:
            logger.error(f"Error collecting network data: {e}")
            data['network']['error'] = str(e)
            
        # Log the datapoint to CSV in simplified format
        self._log_to_csv(data)
        return data
    
    def _log_to_csv(self, data: Dict[str, Any]):
        """Log data point to CSV file in simplified format."""
        try:
            # Extract values from nested structure
            row = [
                data['timestamp'],
                data['sensors'].get('distance_cm', ''),
                data['sensors'].get('motion_detected', ''),
                data['network'].get('connected', {}).get('signal_strength', ''),
                data['network'].get('connected', {}).get('ssid', ''),
                data['network'].get('ping', {}).get('stats', {}).get('avg_ms', ''),
                data['network'].get('ping', {}).get('stats', {}).get('packet_loss_percent', ''),
                data['network'].get('iperf', {}).get('bitrate_mbps', '') 
                if data['network'].get('iperf', {}).get('direction') == 'download' else '',
                data['network'].get('iperf', {}).get('bitrate_mbps', '')
                if data['network'].get('iperf', {}).get('direction') == 'upload' else ''
            ]
            
            with open(self.csv_file, 'a', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(row)
        except Exception as e:
            logger.error(f"Error logging to CSV: {e}")
    
    def start_monitoring(self):
        """Start continuous monitoring in a separate thread."""
        if self.running:
            logger.warning("Monitoring already running")
            return
            
        self.running = True
        self.monitor_thread = threading.Thread(target=self._monitoring_loop)
        self.monitor_thread.daemon = True
        self.monitor_thread.start()
        logger.info("Started continuous monitoring")
    
    def stop_monitoring(self):
        """Stop continuous monitoring."""
        self.running = False
        logger.info("Stopping monitoring...")
    
    def _monitoring_loop(self):
        """Main monitoring loop running in a thread."""
        interval = self.config.get('scan_interval', 60)
        
        while self.running:
            try:
                data = self.collect_single_datapoint()
                logger.info(f"Collected datapoint: distance={data['sensors'].get('distance_cm', 'N/A')}cm, "
                          f"motion={data['sensors'].get('motion_detected', 'N/A')}, "
                          f"signal={data['network'].get('connected', {}).get('signal_strength', 'N/A')}dBm")
                
                # Trigger a full network scan & test if motion is detected
                if data['sensors'].get('motion_detected', False):
                    logger.info("Motion detected! Running full network scan and tests...")
                    # Save scan results
                    scan_file = os.path.join(
                        self.config.get('log_directory', 'logs'),
                        f"scan_{time.strftime('%Y%m%d-%H%M%S')}.json"
                    )
                    self.network_scanner.save_scan_results(scan_file)
                    
                    # Run a more complete performance test
                    if self.config.get('iperf_server'):
                        self.performance_tester.run_full_test_suite(
                            self.config['iperf_server']
                        )
                
                # Sleep until next interval
                time.sleep(interval)
                
            except Exception as e:
                logger.error(f"Error in monitoring loop: {e}")
                time.sleep(5)  # Sleep briefly before retrying
    
    def run_full_analysis(self):
        """Run a comprehensive analysis on demand."""
        logger.info("Starting full analysis")
        
        # Collect multiple sensor readings
        sensor_readings = []
        for _ in range(10):
            try:
                sensor_readings.append({
                    'distance_cm': self.ultrasonic_sensor.distance(),
                    'motion_detected': self.ir_sensor.is_active(),
                    'timestamp': time.time()
                })
                time.sleep(0.5)
            except Exception as e:
                logger.error(f"Error collecting sensor readings: {e}")
        
        # Run network scan
        networks = self.network_scanner.scan_networks()
        
        # Run performance tests
        performance = self.performance_tester.run_full_test_suite(
            self.config.get('iperf_server')
        )
        
        # Save comprehensive results
        results = {
            'timestamp': time.time(),
            'sensor_readings': sensor_readings,
            'networks': [n.to_dict() for n in networks],
            'performance': performance
        }
        
        # Save to JSON file
        filename = os.path.join(
            self.config.get('log_directory', 'logs'),
            f"full_analysis_{time.strftime('%Y%m%d-%H%M%S')}.json"
        )
        
        with open(filename, 'w') as f:
            json.dump(results, f, indent=2)
            
        logger.info(f"Full analysis complete. Results saved to {filename}")
        return filename

def main():
    """Command line interface."""
    parser = argparse.ArgumentParser(description='EnviroNet Analyzer')
    parser.add_argument('--config', default='config.json', help='Path to config file')
    parser.add_argument('--monitor', action='store_true', help='Start continuous monitoring')
    parser.add_argument('--analyze', action='store_true', help='Run a full analysis once')
    parser.add_argument('--test-sensors', action='store_true', help='Test sensor readings')
    parser.add_argument('--test-network', action='store_true', help='Test network scanning')
    
    args = parser.parse_args()
    
    # Create analyzer instance
    analyzer = EnviroNetAnalyzer(args.config)
    
    if args.test_sensors:
        # Test reading from sensors
        print("Testing sensors...")
        for i in range(5):
            distance = analyzer.ultrasonic_sensor.distance()
            motion = analyzer.ir_sensor.is_active()
            print(f"Reading {i+1}: Distance: {distance:.2f}cm, Motion: {motion}")
            time.sleep(1)
        return
        
    if args.test_network:
        # Test network scanning
        print("Testing network scanning...")
        networks = analyzer.network_scanner.scan_networks()
        print(f"Found {len(networks)} networks:")
        for net in networks:
            print(f"  - {net.ssid} ({net.bssid}): Signal {net.signal_strength}dBm, Ch {net.channel}")
        return
    
    if args.analyze:
        # Run a single full analysis
        result_file = analyzer.run_full_analysis()
        print(f"Analysis complete. Results saved to {result_file}")
        return
        
    if args.monitor:
        # Start continuous monitoring
        try:
            analyzer.start_monitoring()
            print("Monitoring started. Press Ctrl+C to stop.")
            # Keep main thread alive
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("Stopping monitoring...")
            analyzer.stop_monitoring()
            time.sleep(2)  # Give time for cleanup
        return
        
    # If no arguments, print help
    parser.print_help()

if __name__ == "__main__":
    main()