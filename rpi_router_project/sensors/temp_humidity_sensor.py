# sensors/temp_humidity_sensor.py
#
# This module simulates a DHT22 (or DHT11) Temperature and Humidity Sensor.
# Monitoring the ambient temperature and humidity is crucial for embedded
# systems like a router. Overheating can lead to performance throttling or
# even hardware failure. High humidity can also be a concern for electronics
# over the long term.
#
# This is a key "industry-relevant" metric to monitor, as thermal management
# is a major consideration in the design and deployment of networking hardware.
#
# This mock implementation will generate realistic temperature and humidity
# fluctuations.
#
# Author: Jules
# Date: 2025-08-13

import random

def read():
    """
    Simulates reading data from the DHT22 sensor and returns it in a
    standardized dictionary format.

    Returns:
        dict: A dictionary containing the sensor data.
              e.g., {'temperature_c': 23.5, 'humidity_percent': 48.2}
    """
    # Real-world implementation would use a library like Adafruit_DHT
    # to read data from the sensor, which handles the specific timing
    # protocol required by DHT sensors.
    # `humidity, temperature = Adafruit_DHT.read_retry(DHT_SENSOR, DHT_PIN)`

    # Simulate typical indoor conditions.
    # Temperature might hover around a baseline (e.g., 22°C) and fluctuate.
    # Let's add a small chance of a "heat spike" to simulate load.
    base_temp = 22.0
    temperature = base_temp + random.uniform(-1.5, 1.5)

    if random.random() < 0.02: # 2% chance of a heat spike
        temperature += random.uniform(5.0, 10.0)

    # Simulate humidity, which is generally more stable indoors.
    base_humidity = 45.0
    humidity = base_humidity + random.uniform(-5.0, 5.0)

    # Clamp values to realistic ranges.
    temperature = round(max(18.0, min(temperature, 40.0)), 1)
    humidity = round(max(30.0, min(humidity, 60.0)), 1)

    return {
        'temperature_c': temperature,
        'humidity_percent': humidity
    }
