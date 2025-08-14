# sensors/ultrasonic_sensor.py
#
# This module simulates an HC-SR04 Ultrasonic Distance Sensor.
# This type of sensor is used to measure distances to objects by sending
# out an ultrasonic pulse and measuring the time it takes for the echo
# to return.
#
# In the context of a router, this is a fantastic way to detect if a large
# object is placed too close to the device, potentially obstructing the
# antennas and degrading the Wi-Fi signal.
#
# This mock implementation will generate random distance data, simulating
# an object being occasionally placed in front of the sensor.
#
# Author: Jules
# Date: 2025-08-13

import random

def read():
    """
    Simulates reading data from the ultrasonic sensor and returns it in a
    standardized dictionary format. The actual obstruction logic is now
    handled in the main loop, which has access to the configuration.

    Returns:
        dict: A dictionary containing the sensor data.
              e.g., {'distance_cm': 25.4}
    """
    # A real implementation would involve a more complex sequence of
    # sending a trigger pulse to the TRIG pin and measuring the echo
    # pulse width on the ECHO pin. This requires precise timing.

    # We simulate that most of the time there's no obstruction,
    # but occasionally an object is placed very close. The threshold
    # for what counts as "very close" is no longer defined here.
    if random.random() < 0.05: # 5% chance of a close object
        # Simulate a distance between 5 and 20 cm.
        distance = random.uniform(5.0, 20.0)
    else:
        # Simulate a clear path, with distances from 20 cm to 200 cm.
        distance = random.uniform(20.1, 200.0)

    return {'distance_cm': distance}
