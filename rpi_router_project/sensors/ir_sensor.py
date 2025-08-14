# sensors/ir_sensor.py
#
# This module simulates an Infrared (IR) proximity sensor.
# In a real-world scenario, an IR sensor can detect the presence of objects
# or people. For this project, we can use it to simulate a simple form
# of "interference" or "activity" near the router.
#
# For example, if a person walks by or an object is placed in front of the
# sensor, it would trigger. This could be correlated with potential dips
# in Wi-Fi performance if the obstruction is significant.
#
# This mock implementation will generate random data to simulate
# the sensor being 'triggered' or 'clear'.
#
# Author: Jules
# Date: 2025-08-13

import random

def read():
    """
    Simulates reading data from the IR sensor and returns it in a
    standardized dictionary format.

    Returns:
        dict: A dictionary containing the sensor data.
              e.g., {'activity_detected': True}
    """
    # In a real implementation, this function would contain the logic
    # to read a GPIO pin connected to the IR sensor.
    # For example, using the RPi.GPIO library:
    # `is_triggered = GPIO.input(IR_PIN) == GPIO.LOW`

    # We simulate a 10% chance of the sensor being triggered at any given time.
    is_triggered = random.random() < 0.1

    return {'activity_detected': is_triggered}
