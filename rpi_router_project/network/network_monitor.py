# network/network_monitor.py
#
# This module simulates a network monitor for the router.
# In a real TP-Link router or a Raspberry Pi configured as one, you would
# use command-line tools to get this information. For example:
# - `iw dev <interface> link` to get signal strength and bitrate.
# - `hostapd_cli all_sta` to get information about connected clients.
#
# This mock implementation will generate plausible-looking data for these
# key performance indicators (KPIs). The goal is to have data that can be
# correlated with the environmental data from the sensors. For example,
# we could simulate a drop in signal strength when the ultrasonic sensor
# detects an obstruction.
#
# Author: Jules
# Date: 2025-08-13

import random

def get_network_stats(is_obstructed=False):
    """
    Simulates fetching key network statistics.

    Args:
        is_obstructed (bool): A flag from the sensor data that might
                              affect network performance.

    Returns:
        dict: A dictionary containing simulated network stats.
    """
    # Simulate signal strength (RSSI - Received Signal Strength Indicator).
    # Measured in dBm, where values closer to 0 are better (e.g., -30 dBm is
    # excellent, -80 dBm is poor).
    base_signal = -40 # A good signal
    signal_strength = base_signal + random.uniform(-5, 5)

    # If the sensor reports an obstruction, we simulate a drop in signal.
    if is_obstructed:
        signal_strength -= random.uniform(10, 20)

    # Simulate the number of connected clients.
    num_clients = random.randint(0, 5)

    return {
        "signal_strength_dbm": round(signal_strength),
        "connected_clients": num_clients,
    }
