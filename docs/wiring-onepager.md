# EnviroNet Analyzer - Hardware Wiring Guide

## Quick Reference

**⚠️ CRITICAL SAFETY NOTE: Always use a logic level shifter between 3.3V Pi and 5V Arduino!**

## Components Required

| Component | Quantity | Notes |
|-----------|----------|-------|
| Raspberry Pi 4 | 1 | 8GB recommended for development |
| Panda AC1200 WiFi | 1 | USB 3.0 dual-band adapter |
| Arduino Uno/Nano | 1 | Any 5V Arduino compatible |
| IR Sensor | 1 | Analog output (e.g., Sharp GP2Y0A21) |
| Ultrasonic Sensor | 1 | HC-SR04 or similar |
| Logic Level Shifter | 1 | **REQUIRED** - Bi-directional 3.3V↔5V |
| Breadboard | 1 | For prototyping |
| Jumper Wires | 10+ | Male-female and male-male |
| Power Supply | 1 | 5V USB-C, 3A+ for Pi |

## Pin Assignments

### Raspberry Pi 4
```
Power:
  - 5V (Pin 2 or 4)     → Logic Level Shifter VCC
  - 3.3V (Pin 1 or 17)  → Logic Level Shifter VCC
  - GND (Pin 6, 9, 14, 20, 25, 30, 34, 39) → Logic Level Shifter GND

I2C:
  - GPIO2 (Pin 3)  → Logic Level Shifter LV1 (3.3V side)
  - GPIO3 (Pin 5)  → Logic Level Shifter LV2 (3.3V side)
```

### Logic Level Shifter
```
3.3V Side (Pi):
  - LV1 → Pi GPIO2 (SDA)
  - LV2 → Pi GPIO3 (SCL)
  - VCC → Pi 3.3V
  - GND → Pi GND

5V Side (Arduino):
  - HV1 → Arduino A4 (SDA)
  - HV2 → Arduino A5 (SCL)
  - VCC → Arduino 5V
  - GND → Arduino GND
```

### Arduino
```
I2C:
  - A4 (SDA) → Logic Level Shifter HV1
  - A5 (SCL) → Logic Level Shifter HV2

Sensors:
  - A0 → IR Sensor analog output
  - D2 → Ultrasonic trigger
  - D3 → Ultrasonic echo
  - 5V → Power sensors
  - GND → Common ground
```

## Wiring Diagram

```
Raspberry Pi 4          Logic Level Shifter          Arduino
┌─────────────┐         ┌─────────────────┐         ┌─────────┐
│             │         │                 │         │         │
│ 3.3V ──────┼─────────┼─ LV (3.3V)     │         │         │
│             │         │                 │         │         │
│ GPIO2 ─────┼─────────┼─ LV1 (SDA)     │         │         │
│             │         │                 │         │         │
│ GPIO3 ─────┼─────────┼─ LV2 (SCL)     │         │         │
│             │         │                 │         │         │
│ GND ───────┼─────────┼─ GND            │         │         │
│             │         │                 │         │         │
└─────────────┘         │                 │         │         │
                        │                 │         │         │
                        │  HV (5V)       │         │         │
                        │                 │         │         │
                        │ HV1 (SDA) ─────┼─────────┼─ A4     │
                        │                 │         │         │
                        │ HV2 (SCL) ─────┼─────────┼─ A5     │
                        │                 │         │         │
                        │ VCC ───────────┼─────────┼─ 5V     │
                        │                 │         │         │
                        │ GND ───────────┼─────────┼─ GND    │
                        └─────────────────┘         │         │
                                                    │         │
                                                    │ A0 ─────┼─ IR Sensor
                                                    │         │
                                                    │ D2 ─────┼─ Ultrasonic Trigger
                                                    │         │
                                                    │ D3 ─────┼─ Ultrasonic Echo
                                                    │         │
                                                    └─────────┘
```

## Step-by-Step Assembly

### 1. Power Off Everything
- Disconnect power from Pi and Arduino
- Remove USB cables

### 2. Connect Logic Level Shifter
- **CRITICAL**: Connect 3.3V side to Pi first
- Connect 5V side to Arduino second
- Double-check all connections before powering on

### 3. Connect I2C Lines
- Pi GPIO2 → Shifter LV1
- Pi GPIO3 → Shifter LV2
- Shifter HV1 → Arduino A4
- Shifter HV2 → Arduino A5

### 4. Connect Power and Ground
- Pi 3.3V → Shifter VCC (3.3V side)
- Pi GND → Shifter GND (3.3V side)
- Arduino 5V → Shifter VCC (5V side)
- Arduino GND → Shifter GND (5V side)

### 5. Connect Sensors
- IR sensor analog → Arduino A0
- Ultrasonic trigger → Arduino D2
- Ultrasonic echo → Arduino D3
- Sensor power → Arduino 5V
- Sensor ground → Arduino GND

### 6. Power On Sequence
1. Power on Arduino first
2. Wait 5 seconds
3. Power on Raspberry Pi
4. Wait for Pi to boot completely

## Testing and Verification

### 1. Check I2C Bus
```bash
# On Pi, check if I2C is enabled
sudo raspi-config
# Interface Options → I2C → Enable

# Check I2C devices
sudo i2cdetect -y 1
# Should show device at address 0x10
```

### 2. Test Arduino Communication
```bash
# Run sensor test
./environet --test-sensors
# Should show sensor frames with valid CRC
```

### 3. Monitor Serial Output
- Connect Arduino to computer via USB
- Open Arduino IDE Serial Monitor (115200 baud)
- Should see sensor readings and I2C requests

## Troubleshooting

### Common Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| No I2C devices found | I2C not enabled | Enable in raspi-config |
| Communication errors | Wrong voltage levels | Check logic level shifter |
| Sensor readings wrong | Bad connections | Recheck all wiring |
| Arduino not responding | Power issues | Check 5V supply |

### Debug Commands
```bash
# Check I2C bus status
sudo i2cdetect -y 1

# Monitor I2C traffic
sudo i2cdump -y 1 0x10

# Check system logs
sudo journalctl -u environet -f

# Test individual components
./environet --test-sensors
./environet --test-network
```

## Safety Warnings

- **NEVER** connect 5V Arduino directly to 3.3V Pi
- **ALWAYS** use logic level shifter for I2C
- Check all connections before powering on
- Use proper power supply (3A+ for Pi)
- Keep wiring neat and organized
- Test with multimeter if unsure

## Next Steps

1. Verify all connections
2. Test I2C communication
3. Run sensor tests
4. Configure network settings
5. Start full monitoring

## Support

- Check logs: `sudo journalctl -u environet`
- Review wiring diagram
- Test individual components
- Consult Arduino and Pi documentation
