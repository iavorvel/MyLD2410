/*
  This program performs a factory reset
  of the HLK-LD2410 presence sensor.

  #define SERIAL_BAUD_RATE sets the serial monitor baud rate

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
//Change the communication baud rate here, if necessary
//#define LD2410_BAUD_RATE 256000
#include "MyLD2410.h"

// User defines
#define SERIAL_BAUD_RATE 115200

MyLD2410 sensor(sensorSerial);

void printValue(const byte &val) {
  Serial.print(' ');
  Serial.print(val);
}

void printParameters() {
  sensor.configMode();
  sensor.requestParameters();
  Serial.print("Firmware: ");
  String fw(sensor.getFirmware());
  Serial.println(fw);
  if (!fw.startsWith(LD2410_LATEST_FIRMWARE)) {
    Serial.print("To get the lastest features, upgrade your firmware to ");
    Serial.println(LD2410_LATEST_FIRMWARE);
  }
  Serial.print("Protocol version: ");
  Serial.println(sensor.getVersion());
  Serial.print("Bluetooth MAC address: ");
  Serial.println(sensor.getMACstr());

  const MyLD2410::ValuesArray &mThr = sensor.getMovingThresholds();
  const MyLD2410::ValuesArray &sThr = sensor.getStationaryThresholds();

  Serial.print("Resolution (gate-width): ");
  Serial.print(sensor.getResolution());
  Serial.print("cm\nMax range: ");
  Serial.print(sensor.getRange_cm());
  Serial.print("cm\nMoving thresholds    [0,");
  Serial.print(mThr.N);
  Serial.print("]:");
  //Print using global function
  mThr.forEach(printValue);
  Serial.print("\nStationary thresholds[0,");
  Serial.print(sThr.N);
  Serial.print("]:");
  //Print using lambda
  sThr.forEach([](const byte &val) {
    Serial.print(' ');
    Serial.print(val);
  });
  Serial.print("\nNo-one window: ");
  Serial.print(sensor.getNoOneWindow());
  Serial.println('s');

  //For firmware >= 2.44
  if (sensor.requestAuxConfig()) {
    Serial.print("Auxiliary Configuration: ");
    switch (sensor.getLightControl()) {
      case LightControl::NO_LIGHT_CONTROL:
        Serial.println("no light control");
        break;
      case LightControl::LIGHT_BELOW_THRESHOLD:
        Serial.println("active when light is below the threshold of ");
        Serial.println(sensor.getLightThreshold());
        break;
      case LightControl::LIGHT_ABOVE_THRESHOLD:
        Serial.println("active when light is above the threshold of ");
        Serial.println(sensor.getLightThreshold());
        break;
      default:
        break;
    }
    switch (sensor.getOutputControl()) {
      case OutputControl::DEFAULT_LOW:
        Serial.println("Default output level: LOW");
        break;
      case OutputControl::DEFAULT_HIGH:
        Serial.println("Default output level: HIGH");
        break;
      default:
        break;
    }
  }
  sensor.configMode(false);
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
#if defined(ARDUINO_XIAO_ESP32C3) || defined(ARDUINO_XIAO_ESP32C6) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_LEONARDO)
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

  printParameters();

  delay(2000);
  Serial.print("\nExecuting factory reset... ");
  if (sensor.setGateParameters(0, 0, 0) && sensor.setGateParameters(1, 0, 0) && sensor.requestReset()) {
    Serial.println("Done!\n");
    printParameters();
  } else Serial.println("Fail\n");
}

void loop() {
}
