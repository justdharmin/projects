#include "HX711.h"

// === PIN MAP ===
#define LOADCELL_DOUT_BABY   8
#define LOADCELL_SCK_BABY    9
#define LOADCELL_DOUT_DRIVER 3
#define LOADCELL_SCK_DRIVER  2
#define PIR_PIN   4
#define SOUND_PIN 6          // Digital Output (DO) from Sound Sensor
#define RELAY_PIN 7          // Active-LOW relay

HX711 babyScale;
HX711 driverScale;

// === CALIBRATION / THRESHOLDS ===
long babyBaseline = 0;
long driverBaseline = 0;
const long STABILITY_MARGIN = 20000;
const unsigned long ALERT_DELAY = 5000; // 5s delay before buzzer

// === STATE HELPERS ===
unsigned long alertStartTime = 0;
bool alertActive = false;

// === PIR SETTINGS ===
bool pirTriggered = false;
unsigned long lastPirTime = 0;
const unsigned long PIR_IGNORE_MS = 300;

// === SOUND SETTINGS ===
unsigned long lastSoundTime = 0;
bool soundDetected = false;

// === FILTER HELPERS (smoothing) ===
byte recentSound[3] = {0, 0, 0};  // store last 3 readings

void setup() {
  Serial.begin(9600);

  babyScale.begin(LOADCELL_DOUT_BABY, LOADCELL_SCK_BABY);
  driverScale.begin(LOADCELL_DOUT_DRIVER, LOADCELL_SCK_DRIVER);

  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // buzzer OFF

  Serial.println("🚗 Baby Presence Alert System Initializing...");
  delay(1500);

  // === CALIBRATION (ensure both seats empty) ===
  Serial.println("🧠 Calibrating: ensure both seats are EMPTY now...");
  delay(2000);

  if (babyScale.is_ready() && driverScale.is_ready()) {
    long bs = 0, ds = 0;
    for (int i = 0; i < 10; i++) {
      bs += babyScale.read();
      ds += driverScale.read();
      delay(150);
    }
    babyBaseline = bs / 10;
    driverBaseline = ds / 10;
  } else {
    Serial.println("⚠️ HX711 not ready for calibration!");
  }

  Serial.print("✅ Calibration done. Driver baseline: ");
  Serial.print(driverBaseline);
  Serial.print(" | Baby baseline: ");
  Serial.println(babyBaseline);
  delay(500);
}

void loop() {
  if (!babyScale.is_ready() || !driverScale.is_ready()) {
    Serial.println("⚠️ HX711 not ready");
    delay(300);
    return;
  }

  // === READ LOAD CELLS ===
  long babyRaw = babyScale.read();
  long driverRaw = driverScale.read();

  long babyDiff = labs(babyRaw - babyBaseline);
  long driverDiff = labs(driverRaw - driverBaseline);

  bool babyPresent = (babyDiff > STABILITY_MARGIN);
  bool driverPresent = (driverDiff > STABILITY_MARGIN);
  bool driverAbsent = !driverPresent;

  // === PIR DETECTION ===
  int pirRaw = digitalRead(PIR_PIN);
  if (pirRaw == HIGH) {
    unsigned long now = millis();
    if (now - lastPirTime > PIR_IGNORE_MS) {
      pirTriggered = true;
      lastPirTime = now;
      Serial.println("PIR: detected (raw HIGH)");
    }
  }

  // === SOUND DETECTION (D6) — smoothed for stability ===
  // Shift old readings
  recentSound[2] = recentSound[1];
  recentSound[1] = recentSound[0];
  recentSound[0] = digitalRead(SOUND_PIN);  // HIGH when sound detected (adjust if inverted)

  // Simple smoothing: if 2 or more of the last 3 reads are HIGH, consider it detected
  int sum = recentSound[0] + recentSound[1] + recentSound[2];
  bool rawDetected = (sum >= 2);

  if (rawDetected) {
    lastSoundTime = millis();
    Serial.println("🎤 Sound detected (smoothed)");
  }

  // Hold active for 1s after last detection
  if (millis() - lastSoundTime < 1000) soundDetected = true;
  else soundDetected = false;

  bool motionDetected = pirTriggered;

  // === DEBUG INFO ===
  Serial.print("DrvDiff: "); Serial.print(driverDiff);
  Serial.print(" | BabyDiff: "); Serial.print(babyDiff);
  Serial.print(" | drvPresent: "); Serial.print(driverPresent);
  Serial.print(" babyPresent: "); Serial.print(babyPresent);
  Serial.print(" | PIR: "); Serial.print(pirRaw);
  Serial.print(" soundDet: "); Serial.print(soundDetected);
  Serial.print(" motionDet: "); Serial.println(motionDetected);

  // === MAIN LOGIC ===
  if (driverAbsent && babyPresent) {
    if (motionDetected || soundDetected) {
      activateBuzzer();
    } else {
      if (alertStartTime == 0) alertStartTime = millis();
      if (millis() - alertStartTime > ALERT_DELAY) activateBuzzer();
    }
  } else {
    deactivateBuzzer();
    alertStartTime = 0;
  }

  pirTriggered = false;
  delay(100); // smoother loop
}

// === BUZZER CONTROL ===
void activateBuzzer() {
  if (!alertActive) {
    digitalWrite(RELAY_PIN, LOW); // Active-LOW
    alertActive = true;
    Serial.println("🚨 ALERT: buzzer ON");
  }
}

void deactivateBuzzer() {
  if (alertActive) {
    digitalWrite(RELAY_PIN, HIGH);
    alertActive = false;
    Serial.println("✅ SAFE: buzzer OFF");
  }
}

