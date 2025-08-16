/*
 * EnviroNet Analyzer - Arduino Sensor Firmware
 * 
 * This Arduino sketch implements an I2C slave that provides sensor data
 * in the format expected by the C++ EnviroNet Analyzer.
 * 
 * Hardware Requirements:
 * - Arduino Uno/Nano or compatible
 * - IR sensor (analog input)
 * - Ultrasonic sensor (HC-SR04 or similar)
 * - I2C communication (SDA/SCL pins)
 * 
 * Wiring:
 * - IR sensor: Analog pin A0
 * - Ultrasonic trigger: Digital pin 2
 * - Ultrasonic echo: Digital pin 3
 * - I2C: SDA (A4), SCL (A5)
 * 
 * I2C Address: 0x10 (16 decimal) - configurable
 * Frame Format: 16 bytes (packed, little-endian)
 *   - uint32_t ts_ms: milliseconds since boot
 *   - int16_t ir_raw: raw ADC reading
 *   - uint16_t ultra_mm: distance in mm
 *   - uint8_t status: status flags
 *   - uint8_t reserved: reserved for future use
 *   - uint16_t crc16: CRC-16-CCITT checksum
 */

#include <Wire.h>

// Configuration
#define I2C_SLAVE_ADDRESS 0x10
#define IR_SENSOR_PIN A0
#define ULTRASONIC_TRIGGER_PIN 2
#define ULTRASONIC_ECHO_PIN 3
#define SAMPLE_INTERVAL_MS 100
#define STATUS_LED_PIN 13

// Status bit definitions
#define STATUS_MOTION 0x01
#define STATUS_ERROR 0x02
#define STATUS_CALIBRATING 0x04
#define STATUS_LOW_BATTERY 0x08

// Sensor frame structure (must match C++ code)
struct SensorFrame {
  uint32_t ts_ms;      // milliseconds since boot
  int16_t ir_raw;      // raw ADC reading
  uint16_t ultra_mm;   // distance in mm
  uint8_t status;      // status flags
  uint8_t reserved;    // reserved
  uint16_t crc16;      // CRC-16-CCITT
} __attribute__((packed));

// Global variables
SensorFrame current_frame;
unsigned long last_sample_time = 0;
bool motion_detected = false;
int error_count = 0;
int calibration_count = 0;

// CRC-16-CCITT calculation
uint16_t calculate_crc16(uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;
  
  for (size_t i = 0; i < length; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    }
  }
  
  return crc;
}

// Read ultrasonic sensor
uint16_t read_ultrasonic() {
  // Clear trigger
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  
  // Send trigger pulse
  digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  
  // Measure echo duration
  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000); // 30ms timeout
  
  if (duration == 0) {
    // Timeout or error
    return 0xFFFF;
  }
  
  // Convert to distance in mm (speed of sound = 340 m/s)
  // distance = (duration * 0.034) / 2 * 1000
  uint16_t distance = (duration * 17) / 1000;
  
  return distance;
}

// Read IR sensor
int16_t read_ir_sensor() {
  int raw_value = analogRead(IR_SENSOR_PIN);
  
  // Convert to signed 16-bit value
  // Assuming 10-bit ADC (0-1023) -> (-512 to 511)
  int16_t signed_value = raw_value - 512;
  
  return signed_value;
}

// Update sensor frame
void update_sensor_frame() {
  unsigned long current_time = millis();
  
  // Check if it's time for a new sample
  if (current_time - last_sample_time >= SAMPLE_INTERVAL_MS) {
    // Read sensors
    int16_t ir_value = read_ir_sensor();
    uint16_t ultra_distance = read_ultrasonic();
    
    // Update status flags
    uint8_t status = 0;
    
    // Check for motion (simple threshold-based detection)
    static int16_t last_ir_value = 0;
    if (abs(ir_value - last_ir_value) > 50) {
      status |= STATUS_MOTION;
      motion_detected = true;
    } else {
      motion_detected = false;
    }
    last_ir_value = ir_value;
    
    // Check for errors
    if (ultra_distance == 0xFFFF) {
      status |= STATUS_ERROR;
      error_count++;
    }
    
    // Check for calibration (first few samples)
    if (calibration_count < 10) {
      status |= STATUS_CALIBRATING;
      calibration_count++;
    }
    
    // Check battery voltage (if available)
    // This is a placeholder - implement actual battery monitoring if needed
    // if (battery_voltage < LOW_BATTERY_THRESHOLD) {
    //   status |= STATUS_LOW_BATTERY;
    // }
    
    // Fill frame
    current_frame.ts_ms = current_time;
    current_frame.ir_raw = ir_value;
    current_frame.ultra_mm = ultra_distance;
    current_frame.status = status;
    current_frame.reserved = 0;
    
    // Calculate CRC (excluding CRC field itself)
    uint8_t *frame_data = (uint8_t*)&current_frame;
    current_frame.crc16 = calculate_crc16(frame_data, sizeof(SensorFrame) - 2);
    
    last_sample_time = current_time;
    
    // Toggle status LED if motion detected
    digitalWrite(STATUS_LED_PIN, motion_detected ? HIGH : LOW);
  }
}

// I2C request handler
void requestEvent() {
  // Send the current sensor frame
  Wire.write((uint8_t*)&current_frame, sizeof(SensorFrame));
}

// I2C receive handler (for commands)
void receiveEvent(int howMany) {
  if (howMany == 1) {
    uint8_t command = Wire.read();
    
    switch (command) {
      case 0x01: // Set sample rate
        if (Wire.available()) {
          uint8_t new_interval = Wire.read();
          // Could implement sample rate change here
        }
        break;
        
      case 0x02: // Reset calibration
        calibration_count = 0;
        error_count = 0;
        break;
        
      case 0x03: // Get status
        // Could send status information back
        break;
        
      default:
        // Unknown command
        break;
    }
  }
}

// Setup function
void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  Serial.println("EnviroNet Sensor Firmware Starting...");
  
  // Initialize I2C as slave
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  // Initialize pins
  pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Initialize sensors
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  // Initial sensor read
  update_sensor_frame();
  
  Serial.println("EnviroNet Sensor Firmware Ready");
  Serial.print("I2C Address: 0x");
  Serial.println(I2C_SLAVE_ADDRESS, HEX);
  Serial.print("Sample Interval: ");
  Serial.print(SAMPLE_INTERVAL_MS);
  Serial.println(" ms");
}

// Main loop
void loop() {
  // Update sensor readings
  update_sensor_frame();
  
  // Print debug info every second
  static unsigned long last_debug_time = 0;
  if (millis() - last_debug_time >= 1000) {
    Serial.print("Time: ");
    Serial.print(current_frame.ts_ms);
    Serial.print(" ms, IR: ");
    Serial.print(current_frame.ir_raw);
    Serial.print(", Ultra: ");
    Serial.print(current_frame.ultra_mm);
    Serial.print(" mm, Status: 0x");
    Serial.print(current_frame.status, HEX);
    Serial.print(", CRC: 0x");
    Serial.println(current_frame.crc16, HEX);
    
    if (motion_detected) {
      Serial.println("*** MOTION DETECTED ***");
    }
    
    if (error_count > 0) {
      Serial.print("Errors: ");
      Serial.println(error_count);
    }
    
    last_debug_time = millis();
  }
  
  // Small delay to prevent overwhelming the system
  delay(10);
}
