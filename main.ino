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
const int modeCount = 6;
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
String inferIntent(String input);
void runBodyTempRoutine();
void runTempMonitoring();

void setup() {
  Serial.begin(115200);
  sensors.begin();
  lcd.begin(16, 2);
  lcd.print("Smart Thermo Ready");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  parseSerialInput();

  // --- Button Press Handling ---
  bool buttonPressedNow = (digitalRead(BUTTON_PIN) == LOW);
  if (buttonPressedNow && !buttonPreviouslyPressed) {
    mode = (mode + 1) % modeCount;
    Serial.print("Mode changed (button): ");
    Serial.println(getModeName(mode));
  }
  buttonPreviouslyPressed = buttonPressedNow;

  // --- Intent Execution ---
  if (mode == 5) {
    runBodyTempRoutine();
    mode = 0; // Reset to default after reading
  } else {
    runTempMonitoring();
  }

  delay(100);
}

// --- Intent Matching ---
String inferIntent(String input) {
  input.toLowerCase();

  if (input.indexOf("orchid") >= 0 || input.indexOf("plant") >= 0 || input.indexOf("flower") >= 0)
    return "orchid";

  if (input.indexOf("baby") >= 0 || input.indexOf("nursery") >= 0 || input.indexOf("crib") >= 0)
    return "babyroom";

  if (input.indexOf("beer") >= 0 || input.indexOf("chill") >= 0 || input.indexOf("drink") >= 0)
    return "beer";

  if (input.indexOf("bedroom") >= 0 || input.indexOf("cozy") >= 0 || input.indexOf("sleep") >= 0)
    return "comfort_bedroom";

  if (input.indexOf("fever") >= 0 || input.indexOf("underarm") >= 0 || input.indexOf("body temp") >= 0 || input.indexOf("temperature check") >= 0)
    return "body_temperature";

  if (input.indexOf("how hot") >= 0 || input.indexOf("how cold") >= 0 || input.indexOf("temperature") >= 0)
    return "check_temp";

  return "default";
}

// --- Serial Input Handler ---
void parseSerialInput() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      inputBuffer.trim();
      inputBuffer.toLowerCase();

      String intent = inferIntent(inputBuffer);

      if (intent == "orchid") mode = 1;
      else if (intent == "beer") mode = 2;
      else if (intent == "babyroom") mode = 3;
      else if (intent == "comfort_bedroom") mode = 4;
      else if (intent == "body_temperature") mode = 5;
      else if (intent == "check_temp") mode = 0;
      else mode = 0;

      Serial.print("[intent: ");
      Serial.print(intent);
      Serial.print("] Mapped to mode: ");
      Serial.println(getModeName(mode));

      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }
}

// --- Temp Monitoring Routine ---
void runTempMonitoring() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  Serial.print("Temp: ");
  Serial.println(tempC);

  bool outOfRange = !inTargetRange(mode, tempC);
  unsigned long now = millis();

  if (outOfRange && !showingAlert) {
    showingAlert = true;
    lastAlertTime = now;
    Serial.print("[");
    Serial.print(now);
    Serial.print("] Context: ");
    Serial.print(getModeName(mode));
    Serial.print(" → Temp: ");
    Serial.print(tempC);
    Serial.println(" °C — ⚠️ OUT OF RANGE");
  }
  if (showingAlert && now - lastAlertTime > 1000) {
    showingAlert = false;
  }

  // LCD Update
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
}

// --- Body Temp Routine (Underarm) ---
void runBodyTempRoutine() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place under arm");
  lcd.setCursor(0, 1);
  lcd.print("Stabilizing...");

  Serial.println("[BodyTemp] Starting measurement...");

  float lastTemp = 0.0;
  int stableCounter = 0;
  const float threshold = 0.1;
  const int requiredStableSeconds = 5;

  for (int i = 0; i < 60; i++) {
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);

    Serial.print("[");
    Serial.print(i + 1);
    Serial.print("s] Temp: ");
    Serial.println(temp);

    if (i > 0 && abs(temp - lastTemp) < threshold) {
      stableCounter++;
      Serial.println("↳ Stable reading");
    } else {
      stableCounter = 0;
    }

    if (stableCounter >= requiredStableSeconds) {
      Serial.println("✅ Temperature stabilized.");
      break;
    }

    lastTemp = temp;
    delay(1000);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Final Temp:");
  lcd.setCursor(0, 1);
  lcd.print(lastTemp, 1);
  lcd.print(" C");

  // Prepare result lines
  char line1[17];
  snprintf(line1, sizeof(line1), "Final Temp: %.1fC", lastTemp);

  String line2; 
  if (lastTemp >= 37.5) {
    Serial.println("⚠️ Fever detected");
    line2 = "Fever Detected";
  } else if (lastTemp < 35.5) {
    Serial.println("⚠️ Low temperature");
    line2 = "Too Cold!";
  } else {
    Serial.println("✅ Normal temperature");
    line2 = "Normal";
  }

  // Blink result 3 times (6 sec total)
  for (int i = 0; i < 3; i++) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    delay(1000);

    lcd.clear();  // blank screen
    delay(500);
  }
}

// --- Helpers ---
String getModeName(int m) {
  switch (m) {
    case 0: return "Default";
    case 1: return "Orchid";
    case 2: return "Beer";
    case 3: return "BabyRoom";
    case 4: return "Bedroom";
    case 5: return "BodyTemp";
    default: return "Unknown";
  }
}

void getTargetRange(int m, float &low, float &high) {
  switch (m) {
    case 0: low = 15; high = 30; break;       // Default
    case 1: low = 18; high = 24; break;       // Orchid
    case 2: low = 3;  high = 6;  break;       // Beer
    case 3: low = 20; high = 25; break;       // BabyRoom
    case 4: low = 19; high = 24; break;       // Bedroom
    case 5: low = 36.1; high = 37.2; break;   // BodyTemp
    default: low = 0; high = 100;
  }
}

bool inTargetRange(int m, float temp) {
  float low, high;
  getTargetRange(m, low, high);
  return temp >= low && temp <= high;
}
