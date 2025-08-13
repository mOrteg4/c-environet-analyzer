import random
import time
from typing import Callable, Optional

class MockIRSensor:
    """Mimics a digital IR sensor that detects presence/absence."""
    
    def __init__(self, 
                 pin: int,
                 detection_probability: float = 0.2,
                 state_change_delay: float = 2.0,
                 pattern: Optional[Callable] = None):
        self.pin = pin
        self.detection_probability = detection_probability
        self.state_change_delay = state_change_delay
        self._last_state_change = time.time()
        self._current_state = 0  # 0 = no detection, 1 = detection
        self._pattern = pattern or self._default_pattern
        
    def _default_pattern(self, current_time: float) -> int:
        """Default behavior: random state changes with minimum duration"""
        # Check if enough time has passed since last state change
        if current_time - self._last_state_change > self.state_change_delay:
            # Randomly decide to change state based on probability
            if random.random() < self.detection_probability:
                self._current_state = 1 if self._current_state == 0 else 0
                self._last_state_change = current_time
        
        return self._current_state
    
    def is_active(self) -> bool:
        """Check if sensor is detecting anything."""
        state = self._pattern(time.time())
        # Simulate read delay
        time.sleep(0.005)
        return bool(state)
    
    def get_mock_data_stream(self, duration_seconds: int, interval: float = 0.1):
        """Generate a time series of readings over the specified duration."""
        readings = []
        start_time = time.time()
        current_time = start_time
        
        while current_time - start_time < duration_seconds:
            readings.append({
                'timestamp': current_time,
                'detection': self.is_active(),
            })
            time.sleep(interval)
            current_time = time.time()
            
        return readings