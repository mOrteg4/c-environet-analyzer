import random
import time
from typing import Callable, Optional

class MockUltrasonicSensor:
    """Mimics a real ultrasonic sensor's behavior with configurable patterns."""
    
    def __init__(self, 
                 trigger_pin: int, 
                 echo_pin: int,
                 min_distance: float = 2.0,
                 max_distance: float = 400.0,
                 noise_factor: float = 0.05,
                 pattern: Optional[Callable] = None):
        self.trigger_pin = trigger_pin
        self.echo_pin = echo_pin
        self.min_distance = min_distance
        self.max_distance = max_distance
        self.noise_factor = noise_factor
        self._current_distance = 100.0  # Default starting distance in cm
        self._pattern = pattern or self._default_pattern
        
    def _default_pattern(self, current_time: float) -> float:
        """Default behavior: random walk with noise"""
        # Slight random walk
        self._current_distance += random.uniform(-5, 5)
        # Constrain to valid range
        self._current_distance = max(self.min_distance, 
                                    min(self._current_distance, 
                                        self.max_distance))
        # Add noise
        noise = random.uniform(-self._current_distance * self.noise_factor,
                              self._current_distance * self.noise_factor)
        return self._current_distance + noise
    
    def distance(self) -> float:
        """Get current distance reading in cm."""
        # Call pattern function with current time
        distance = self._pattern(time.time())
        # Simulate sensor read delay
        time.sleep(0.01)
        return distance
    
    def get_mock_data_stream(self, duration_seconds: int, interval: float = 0.1):
        """Generate a time series of readings over the specified duration."""
        readings = []
        start_time = time.time()
        current_time = start_time
        
        while current_time - start_time < duration_seconds:
            readings.append({
                'timestamp': current_time,
                'distance_cm': self.distance(),
            })
            time.sleep(interval)
            current_time = time.time()
            
        return readings