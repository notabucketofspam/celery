#include <Arduino.h>
#include <AccelStepper.h>
#include <stdlib.h>
#include <string.h>

// use Serial if the pi and arduino are talking via a usb cable
// use Serial1 if Pi pin 32 (uart5) is connected to Arduino pin D19
#define WHICH_SERIAL Serial1

// RAMPS 1.4 Stepper Pins
#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38

#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56

AccelStepper stepperPan(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperTilt(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);

const byte numChars = 32;
char receivedChars[numChars];
bool newData = false;

// Target vs. Current speeds for smooth mechanical acceleration
float targetPanSpeed = 0.0f;
float targetTiltSpeed = 0.0f;
float currentPanSpeed = 0.0f;
float currentTiltSpeed = 0.0f;

// Maximum speed change per millisecond (12.0 steps/ms = 12,000 steps/sec^2)
const float MAX_ACCEL_PER_MS = 12.0f;

unsigned long lastPacketTime = 0;
unsigned long lastAccelUpdate = 0;
const unsigned long WATCHDOG_TIMEOUT_MS = 250;

void recvWithStartEndMarkers() {
  static bool recvInProgress = false;
  static byte ndx = 0;
  const char startMarker = '<';
  const char endMarker = '>';

  while (WHICH_SERIAL.available() > 0 && !newData) {
    char rc = WHICH_SERIAL.read();

    if (recvInProgress) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars[ndx] = '\0';
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    } else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void parseData() {
  char* strtokIndx = strtok(receivedChars, ",");
  if (!strtokIndx) return;
  targetPanSpeed = static_cast<float>(atoi(strtokIndx));

  strtokIndx = strtok(NULL, ",");
  if (!strtokIndx) return;
  targetTiltSpeed = static_cast<float>(atoi(strtokIndx));
}

float stepToward(float current, float target, float maxStep) {
  if (target > current) {
    return min(current + maxStep, target);
  } else if (target < current) {
    return max(current - maxStep, target);
  }
  return current;
}

void setup() {
  WHICH_SERIAL.begin(115200);

  pinMode(X_ENABLE_PIN, OUTPUT);
  digitalWrite(X_ENABLE_PIN, LOW);

  pinMode(Y_ENABLE_PIN, OUTPUT);
  digitalWrite(Y_ENABLE_PIN, LOW);

  stepperPan.setMaxSpeed(4000);
  stepperTilt.setMaxSpeed(4000);

  stepperPan.setSpeed(0);
  stepperTilt.setSpeed(0);
}

void loop() {
  recvWithStartEndMarkers();
  if (newData) {
    parseData();
    newData = false;
    lastPacketTime = millis();
  }

  unsigned long now = millis();

  // Safety watchdog: ramp down to zero if Pi stops sending packets
  if (now - lastPacketTime > WATCHDOG_TIMEOUT_MS) {
    targetPanSpeed = 0.0f;
    targetTiltSpeed = 0.0f;
  }

  // 1000 Hz smooth velocity ramp to prevent stepper rotor stalls
  if (now - lastAccelUpdate >= 1) {
    float dt_ms = static_cast<float>(now - lastAccelUpdate);
    lastAccelUpdate = now;

    float maxDelta = MAX_ACCEL_PER_MS * dt_ms;
    currentPanSpeed = stepToward(currentPanSpeed, targetPanSpeed, maxDelta);
    currentTiltSpeed = stepToward(currentTiltSpeed, targetTiltSpeed, maxDelta);

    stepperPan.setSpeed(currentPanSpeed);
    stepperTilt.setSpeed(currentTiltSpeed);
  }

  stepperPan.runSpeed();
  stepperTilt.runSpeed();
}
