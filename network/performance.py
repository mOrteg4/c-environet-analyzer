import subprocess
import re
import json
import logging
import time
from typing import Dict, Any, Optional, List, Tuple
import os

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    filename='network_performance.log'
)
logger = logging.getLogger('network.performance')

class NetworkPerformanceTester:
    """Tests network performance using various metrics."""
    
    def __init__(self, interface: str = 'wlan0'):
        self.interface = interface
        logger.info(f"Initialized NetworkPerformanceTester on interface {interface}")
    
    def ping_test(self, target: str = '8.8.8.8', count: int = 10) -> Dict[str, Any]:
        """
        Run a ping test to measure latency.
        
        Args:
            target: IP address or hostname to ping
            count: Number of pings to send
            
        Returns:
            Dictionary with ping statistics
        """
        logger.info(f"Starting ping test to {target} ({count} pings)")
        try:
            result = subprocess.run(
                ['ping', '-c', str(count), '-I', self.interface, target],
                capture_output=True,
                text=True,
                check=True
            )
            
            # Parse ping statistics
            output = result.stdout
            
            # Extract min/avg/max/mdev values
            stats_match = re.search(
                r'min/avg/max/mdev = ([0-9.]+)/([0-9.]+)/([0-9.]+)/([0-9.]+)', 
                output
            )
            
            if stats_match:
                stats = {
                    'min_ms': float(stats_match.group(1)),
                    'avg_ms': float(stats_match.group(2)),
                    'max_ms': float(stats_match.group(3)),
                    'mdev_ms': float(stats_match.group(4))
                }
            else:
                stats = {}
                
            # Extract packet loss
            loss_match = re.search(r'([0-9.]+)% packet loss', output)
            if loss_match:
                stats['packet_loss_percent'] = float(loss_match.group(1))
                
            logger.info(f"Ping test complete: avg={stats.get('avg_ms', 'N/A')}ms, " 
                      f"loss={stats.get('packet_loss_percent', 'N/A')}%")
            return {
                'timestamp': time.time(),
                'target': target,
                'interface': self.interface,
                'stats': stats
            }
            
        except subprocess.SubprocessError as e:
            logger.error(f"Error during ping test: {e}")
            return {
                'timestamp': time.time(),
                'target': target,
                'interface': self.interface,
                'error': str(e)
            }
    
    def iperf_test(self, server: str, port: int = 5201, 
                  duration: int = 10, direction: str = 'download') -> Dict[str, Any]:
        """
        Run an iperf3 throughput test.
        
        Args:
            server: iperf3 server address
            port: iperf3 server port
            duration: test duration in seconds
            direction: 'download' or 'upload'
            
        Returns:
            Dictionary with throughput results
        """
        logger.info(f"Starting iperf3 {direction} test to {server}:{port} for {duration}s")
        
        cmd = ['iperf3', '-c', server, '-p', str(port), '-t', str(duration), '-J']
        
        # Add specific flags for upload/download
        if direction == 'upload':
            cmd.append('-R')
            
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=True
            )
            
            # Parse JSON output
            data = json.loads(result.stdout)
            
            # Extract key metrics
            if 'end' in data:
                summary = data['end']
                if 'sum_received' in summary:
                    bitrate = summary['sum_received']['bits_per_second']
                    bytes_transferred = summary['sum_received']['bytes']
                elif 'sum_sent' in summary:
                    bitrate = summary['sum_sent']['bits_per_second']
                    bytes_transferred = summary['sum_sent']['bytes']
                else:
                    bitrate = 0
                    bytes_transferred = 0
                    
                mbps = bitrate / 1_000_000  # Convert to Mbps
                
                results = {
                    'timestamp': time.time(),
                    'server': server,
                    'port': port,
                    'interface': self.interface,
                    'direction': direction,
                    'duration_seconds': duration,
                    'bitrate_mbps': mbps,
                    'bytes_transferred': bytes_transferred
                }
                
                logger.info(f"iperf3 {direction} test complete: {mbps:.2f} Mbps")
                return results
            else:
                logger.error("Unexpected iperf3 output format")
                return {
                    'timestamp': time.time(),
                    'server': server,
                    'error': 'Unexpected output format'
                }
                
        except subprocess.SubprocessError as e:
            logger.error(f"Error during iperf3 test: {e}")
            return {
                'timestamp': time.time(),
                'server': server,
                'interface': self.interface,
                'error': str(e)
            }
        except json.JSONDecodeError as e:
            logger.error(f"Error parsing iperf3 JSON output: {e}")
            return {
                'timestamp': time.time(),
                'server': server,
                'interface': self.interface,
                'error': f"JSON parse error: {str(e)}"
            }
    
    def run_full_test_suite(self, iperf_server: Optional[str] = None) -> Dict[str, Any]:
        """
        Run a complete set of network tests.
        
        Args:
            iperf_server: Optional iperf3 server address
            
        Returns:
            Dictionary with all test results
        """
        results = {
            'timestamp': time.time(),
            'interface': self.interface
        }
        
        # Ping tests to multiple targets
        ping_targets = ['8.8.8.8', '1.1.1.1', 'google.com']
        ping_results = {}
        
        for target in ping_targets:
            ping_results[target] = self.ping_test(target)
        
        results['ping_tests'] = ping_results
        
        # iperf tests if server is specified
        if iperf_server:
            results['download_test'] = self.iperf_test(
                iperf_server, direction='download'
            )
            results['upload_test'] = self.iperf_test(
                iperf_server, direction='upload'
            )
            
        # Save results
        timestamp = time.strftime("%Y%m%d-%H%M%S")
        filename = f"network_test_{timestamp}.json"
        
        # Create directory if it doesn't exist
        os.makedirs('logs', exist_ok=True)
        filepath = os.path.join('logs', filename)
        
        with open(filepath, 'w') as f:
            json.dump(results, f, indent=2)
            
        logger.info(f"Saved full test results to {filepath}")
        return results