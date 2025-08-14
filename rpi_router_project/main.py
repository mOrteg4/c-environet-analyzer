# main.py
#
# This script serves as the main entry point for the Raspberry Pi Router Monitoring System.
# It orchestrates the collection of data from various sensors and network monitors,
# processes the data, and provides a consolidated output.
#
# The primary goal is to create a holistic view of the router's operational status,
# combining environmental data (like temperature, humidity, and physical obstructions)
# with network performance metrics (like signal strength and client connections).
#
# This refined version uses a central configuration file for settings and standardized
# data structures (dictionaries) from sensors, representing a more robust and
# professional software architecture.
#
# Author: Jules
# Date: 2025-08-13

import time
import configparser
from datetime import datetime

# Import the sensor and network modules from their respective packages
from sensors import ir_sensor, ultrasonic_sensor, temp_humidity_sensor
from network import network_monitor

def main():
    """
    Main function to run the router monitoring system.

    This function reads configuration, then runs an infinite loop to gather
    data from simulated sources, process it, and print a formatted status line.
    """
    # 1. LOAD CONFIGURATION
    # ==========================================
    config = configparser.ConfigParser()
    # Note: The path is relative to the project root where this script is executed from.
    config.read('rpi_router_project/config/settings.ini')
    obstruction_threshold = config.getfloat('Sensors', 'obstruction_threshold_cm')

    print("Initializing Router Monitoring System...")
    print(f"Configuration loaded: Obstruction threshold set to {obstruction_threshold} cm.")
    print("Press Ctrl+C to exit.")
    print("-" * 80)

    try:
        while True:
            # 2. GATHER DATA FROM SENSORS AND NETWORK
            # ==========================================

            # Read from environmental sensors (each returns a dictionary)
            ir_data = ir_sensor.read()
            ultrasonic_data = ultrasonic_sensor.read()
            temp_humidity_data = temp_humidity_sensor.read()

            # 3. PROCESS AND INTEGRATE DATA
            # ==========================================

            # Determine obstruction status based on config
            distance_cm = ultrasonic_data['distance_cm']
            is_obstructed_flag = distance_cm < obstruction_threshold

            # Get network stats, feeding in the obstruction status
            net_stats = network_monitor.get_network_stats(is_obstructed=is_obstructed_flag)

            # 4. DISPLAY DATA
            # ==========================================

            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

            # Extract data for formatting
            ir_status = "DETECTED" if ir_data['activity_detected'] else "Clear"
            obstruction_status = "YES" if is_obstructed_flag else "NO"
            temp_c = temp_humidity_data['temperature_c']
            humidity_percent = temp_humidity_data['humidity_percent']
            signal_strength = net_stats["signal_strength_dbm"]
            clients = net_stats["connected_clients"]

            status_line = (
                f"[{timestamp}] "
                f"Signal: {signal_strength} dBm | "
                f"Clients: {clients} | "
                f"Temp: {temp_c}°C | "
                f"Humidity: {humidity_percent}% | "
                f"Obstruction: {obstruction_status} ({distance_cm:.1f} cm) | "
                f"IR Activity: {ir_status}"
            )

            print(status_line, flush=True)
            time.sleep(2)

    except KeyboardInterrupt:
        # Handle Ctrl+C gracefully
        print("\n" + "-" * 80)
        print("Router Monitoring System shutting down.")
    except Exception as e:
        # Catch other potential errors, like a missing config file
        print(f"\nAn unexpected error occurred: {e}")

if __name__ == "__main__":
    main()
