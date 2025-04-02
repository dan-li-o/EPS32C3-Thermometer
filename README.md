# EPS32C3-Thermometer

Universal Smart Thermometer (MVP) Documentation

✨ Overview

This is the MVP (Minimum Viable Product) for a modular AI-enabled Universal Smart Thermometer, built with:

ESP32-C3 SuperMini

DS18B20 Waterproof Temperature Sensor

16x2 LCD Display (without I2C backpack)

Push Button

The thermometer supports multiple modes like "Default", "Orchid Care", "Beer Cooling", and "Baby Room Monitoring". It displays the current temperature and alerts the user if the temperature is out of the specified range.

📚 Parts List

Component

Quantity

Notes

ESP32-C3 SuperMini

1

Microcontroller board

DS18B20 Temp Sensor

1

Waterproof, 3-wire, with pull-up resistor

16x2 LCD (no I2C)

1

RS, E, D4-D7 manually wired

Push Button

1

For cycling through modes

4.7kΩ - 10kΩ Resistor

1

Between VCC and data pin of DS18B20

10kΩ Potentiometer

1

For adjusting LCD contrast (between VO and GND with VCC on wiper)

Jumper Wires / Breadboard

As needed

For all connections

🔹 Wiring Diagram

**DS18B20 Temperature Sensor**

Red (VCC): 3.3V on ESP32-C3

Black (GND): GND on ESP32-C3

Yellow (Data): GPIO 4

5.1kΩ resistor between Data and VCC

**16x2 LCD Display (LiquidCrystal Pins)**

LCD   -> ESP32-C3
--------------------
RS    -> GPIO 7
E     -> GPIO 8
D4    -> GPIO 9
D5    -> GPIO 10
D6    -> GPIO 20
D7    -> GPIO 21
VSS   -> GND
VDD   -> 5V
RW    -> GND
VO    -> Wiper of 10kΩ potentiometer
          (other legs of potentiometer to GND and 5V)
A     -> 5V (Backlight +)
K     -> GND (Backlight -)

Push Button

One leg to GPIO 2

Other leg to GND

Internal pull-up resistor enabled in code

📓 Version

MVP working as of: April 2025

Contributors: Dan + GPT Hardware Engineer :)

