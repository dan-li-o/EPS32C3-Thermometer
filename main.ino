#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

// LCD Pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 20, 21);

#define ONE_WIRE_BUS 4  // DS18B20 data line
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define BUTTON_PIN 2    // Push button

int mode = 0;
const int modeCount = 4;
bool buttonPreviouslyPressed = false;

unsigned long lastDisplayTime = 0;
unsigned long lastAlertTime = 0;
bool showingAlert = false;

String inputBuffer = ""; // Buffer for serial input

// --- Function Prototypes ---
String getModeName(int m);
bool inTargetRange(int m, float temp);
void getTargetRange(int m, float &low, float &high);
void parseSerialInput();

void setup() {
  Serial.begin(115200);
  sensors.begin();
  lcd.begin(16, 2);
  lcd.print("Smart Thermo Ready");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  // --- Check for Serial Input ---
  parseSerialInput();

  // --- Check button press to cycle mode ---
  bool buttonPressedNow = (digitalRead(BUTTON_PIN) == LOW);
  if (buttonPressedNow && !buttonPreviouslyPressed) {
    mode = (mode + 1) % modeCount;
    Serial.print("Mode changed (button): ");
    Serial.println(getModeName(mode));
  }
  buttonPreviouslyPressed = buttonPressedNow;

  // --- Read temperature ---
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.print("Temp: ");
  Serial.println(tempC);

  bool outOfRange = !inTargetRange(mode, tempC);
  unsigned long now = millis();

  if (outOfRange && !showingAlert) {
    showingAlert = true;
    lastAlertTime = now;
    Serial.println("⚠️ Temperature out of range");
  }
  if (showingAlert && now - lastAlertTime > 1000) {
    showingAlert = false;
  }

  // --- Update LCD ---
  if (now - lastDisplayTime > 500) {
    lcd.clear();
    if (showingAlert) {
      lcd.setCursor(0, 0);
      lcd.print("Alert!");
      lcd.setCursor(0, 1);
      float low, high;
      getTargetRange(mode, low, high);
      if (tempC < low) lcd.print("Too Cold!");
      else if (tempC > high) lcd.print("Too Hot!");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Mode: ");
      lcd.print(getModeName(mode));
      lcd.setCursor(0, 1);
      lcd.print("Temp: ");
      lcd.print(tempC, 1);
      lcd.print(" C");
    }
    lastDisplayTime = now;
  }

  delay(100);
}

// --- Serial Input Parser ---
void parseSerialInput() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      inputBuffer.trim();
      inputBuffer.toLowerCase();

      if (inputBuffer.indexOf("orchid") >= 0) mode = 1;
      else if (inputBuffer.indexOf("beer") >= 0) mode = 2;
      else if (inputBuffer.indexOf("baby") >= 0) mode = 3;
      else mode = 0;

      Serial.print("Mode changed (text): ");
      Serial.println(getModeName(mode));

      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }
}

// --- Mode name helper ---
String getModeName(int m) {
  switch (m) {
    case 0: return "Default";
    case 1: return "Orchid";
    case 2: return "Beer";
    case 3: return "BabyRoom";
    default: return "Unknown";
  }
}

// --- Get thresholds per mode ---
void getTargetRange(int m, float &low, float &high) {
  switch (m) {
    case 0: low = 15; high = 30; break;
    case 1: low = 18; high = 24; break;
    case 2: low = 3;  high = 6;  break;
    case 3: low = 20; high = 25; break;
    default: low = 0; high = 100;
  }
}

// --- Check range ---
bool inTargetRange(int m, float temp) {
  float low, high;
  getTargetRange(m, low, high);
  return temp >= low && temp <= high;
}
