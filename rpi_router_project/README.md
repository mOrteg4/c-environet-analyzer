# Raspberry Pi Router Environmental Monitor

## 1. Project Goal

This project is a simulation of an advanced monitoring system for a Raspberry Pi-based router. The primary goal is to demonstrate a key concept in embedded systems and networking: **correlating environmental factors with network performance.**

It showcases an understanding of Python, software architecture, and the logic required to interface with GPIO sensors in a networking context. The system is designed to be a robust foundation for a real-world hardware project.

The core of the project is a monitoring script that gathers data from simulated environmental sensors and a simulated network interface, then presents a unified view of the router's status.

## 2. Architecture

The project is organized into a clean, modular structure to promote maintainability and scalability.

-   `main.py`: The main entry point of the application. It handles configuration, orchestrates data collection, and prints the final status.
-   `config/`: Contains configuration files.
    -   `settings.ini`: A central configuration file for easily tweaking parameters (like sensor thresholds) without changing the code.
    -   `router_config_notes.sh`: Detailed, commented instructions for setting up a Raspberry Pi as a router in a real-world scenario.
-   `sensors/`: A Python package containing all sensor-related modules. Each sensor is simulated in its own file and returns data in a standardized dictionary format.
    -   `ir_sensor.py`: Simulates an IR sensor for detecting presence/activity.
    -   `ultrasonic_sensor.py`: Simulates an ultrasonic sensor for detecting physical obstructions.
    -   `temp_humidity_sensor.py`: Simulates a DHT22 sensor for monitoring temperature and humidity, crucial for device health.
-   `network/`: A Python package for network-related logic.
    -   `network_monitor.py`: Simulates a network interface, providing metrics like signal strength and connected clients. Crucially, its output is influenced by data from the sensors (e.g., an obstruction lowers signal strength).

## 3. How to Run

1.  **Prerequisites:** Ensure you have Python 3 installed. No external libraries are needed.
2.  **Execution:** Navigate to the root directory of the repository and run the main script as a module. This ensures that the Python path is set up correctly to find the packages.

    ```bash
    python3 -m rpi_router_project.main
    ```

3.  **Expected Output:** The script will continuously print a new status line to the console every two seconds. The output will look like this:

    ```
    [2025-08-13 18:00:00] Signal: -42 dBm | Clients: 3 | Temp: 22.5°C | Humidity: 45.1% | Obstruction: NO (35.7 cm) | IR Activity: Clear
    ```

4.  **Stopping the script:** Press `Ctrl+C` to gracefully shut down the monitor.

## 4. Configuration

The system's behavior can be modified by editing `rpi_router_project/config/settings.ini`.

-   **`obstruction_threshold_cm`**: This value in the `[Sensors]` section determines how close an object needs to be (in centimeters) for the system to consider it a physical obstruction. This is a great example of how to make your projects flexible.
