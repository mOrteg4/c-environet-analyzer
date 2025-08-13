"""
Interface layer that abstracts whether we're using real or mock sensors.
This makes it easy to swap between testing and production.
"""

import os
from typing import Union, Dict, Any

# Flag to determine if we're using real hardware or mocks
USE_REAL_HARDWARE = os.environ.get('USE_REAL_HARDWARE', '0') == '1'

# Import appropriate implementation based on flag
if USE_REAL_HARDWARE:
    try:
        import RPi.GPIO as GPIO
        from gpiozero import DistanceSensor, MotionSensor
        # Real implementations would go here
    except ImportError:
        print("Warning: GPIO libraries not available. Falling back to mocks.")
        USE_REAL_HARDWARE = False

# Always import mocks for testing purposes
from .ultrasonic_mock import MockUltrasonicSensor
from .ir_mock import MockIRSensor

class SensorFactory:
    """Factory to create appropriate sensor implementation."""
    
    @staticmethod
    def create_ultrasonic_sensor(trigger_pin: int, echo_pin: int, **kwargs) -> Union[Any, MockUltrasonicSensor]:
        """Create ultrasonic distance sensor."""
        if USE_REAL_HARDWARE:
            # Return real implementation
            return DistanceSensor(echo=echo_pin, trigger=trigger_pin, **kwargs)
        else:
            # Return mock implementation
            return MockUltrasonicSensor(trigger_pin=trigger_pin, echo_pin=echo_pin, **kwargs)
    
    @staticmethod
    def create_ir_sensor(pin: int, **kwargs) -> Union[Any, MockIRSensor]:
        """Create IR motion/presence sensor."""
        if USE_REAL_HARDWARE:
            # Return real implementation
            return MotionSensor(pin, **kwargs)
        else:
            # Return mock implementation
            return MockIRSensor(pin=pin, **kwargs)