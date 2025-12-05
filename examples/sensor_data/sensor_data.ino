/*
  This program reads all data received from
  the HLK-LD2410 presence sensor and periodically
  prints the values to the serial monitor.

  Several #defines control the behavior of the program:
  #define SERIAL_BAUD_RATE sets the serial monitor baud rate

  #define ENHANCED_MODE enables the enhanced (engineering)
  mode of the sensor. Comment that line to switch to basic mode.

  #define DEBUG_MODE enables the printing of debug information
  (all received frames are printed). Comment the line to disable
  debugging.

  Communication with the sensor is handled by the
  "MyLD2410" library Copyright (c) Iavor Veltchev 2024

  Use only hardware UART at the default baud rate 256000,
  or change the #define LD2410_BAUD_RATE to match your sensor.
  For ESP32 or other boards that allow dynamic UART pins,
  modify the RX_PIN and TX_PIN defines in "./board_select.h"

  Connection diagram:
  Arduino/ESP32 RX  -- TX LD2410
  Arduino/ESP32 TX  -- RX LD2410
  Arduino/ESP32 GND -- GND LD2410
  Provide sufficient power to the sensor Vcc (200mA, 5-12V)
*/
#include "./board_select.h"
// Change the communication baud rate here, if previously configured
// #define LD2410_BAUD_RATE 256000
#include "MyLD2410.h"

// User defines
// #define DEBUG_MODE
#define ENHANCED_MODE
#define SERIAL_BAUD_RATE 115200


#ifdef DEBUG_MODE
MyLD2410 sensor(sensorSerial, true);
#else
MyLD2410 sensor(sensorSerial);
#endif

const unsigned long printEvery = 1000;  // print every second

void printValue(const byte &val) {
  Serial.print(' ');
  Serial.print(val);
}

void printData() {
  Serial.print("Data frame #: ");
  Serial.println(sensor.getFrameCount());
  Serial.print("Time stamp [ms]: ");
  Serial.println(sensor.getTimestamp());
  Serial.print(sensor.statusString());
  if (sensor.presenceDetected()) {
    Serial.print(", distance: ");
    Serial.print(sensor.detectedDistance());
    Serial.print("cm");
  }
  Serial.println();
  if (sensor.movingTargetDetected()) {
    Serial.print(" MOVING    = ");
    Serial.print(sensor.movingTargetSignal());
    Serial.print("@");
    Serial.print(sensor.movingTargetDistance());
    Serial.print("cm ");
    if (sensor.inEnhancedMode()) {
      Serial.print("\n signals->[");
      sensor.getMovingSignals().forEach(printValue);
      Serial.print(" ] thresholds:[");
      sensor.getMovingThresholds().forEach(printValue);
      Serial.print(" ]");
    }
    Serial.println();
  }
  if (sensor.stationaryTargetDetected()) {
    Serial.print(" STATIONARY= ");
    Serial.print(sensor.stationaryTargetSignal());
    Serial.print("@");
    Serial.print(sensor.stationaryTargetDistance());
    Serial.print("cm ");
    if (sensor.inEnhancedMode()) {
      Serial.print("\n signals->[");
      sensor.getStationarySignals().forEach(printValue);
      Serial.print(" ] thresholds:[");
      sensor.getStationaryThresholds().forEach(printValue);
      Serial.print(" ]");
    }
    Serial.println();
  }

  if (sensor.inEnhancedMode() && (sensor.getFirmwareMajor() > 1)) {
    Serial.print("Light level: ");
    Serial.println(sensor.getLightLevel());
    Serial.print("Output level: ");
    Serial.println((sensor.getOutLevel()) ? "HIGH" : "LOW");
  }

  Serial.println();
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
#if defined(ARDUINO_XIAO_ESP32C3) || defined(ARDUINO_XIAO_ESP32C6) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
  sensorSerial.begin(LD2410_BAUD_RATE);
#else
  sensorSerial.begin(LD2410_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
#endif
  delay(2000);
  Serial.println(__FILE__);
  if (!sensor.begin()) {
    Serial.println("Failed to communicate with the sensor.");
    while (true) {}
  }

#ifdef ENHANCED_MODE
  sensor.enhancedMode();
#else
  sensor.enhancedMode(false);
#endif

  delay(printEvery);
}

void loop() {
  static unsigned long nextPrint = 0;
  if ((sensor.check() == MyLD2410::Response::DATA) && (millis() >= nextPrint)) {
    nextPrint = millis() + printEvery;
    printData();
  }
}
