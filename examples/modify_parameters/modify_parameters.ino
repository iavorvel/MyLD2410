/*
  This program demonstrates how to modify the setup parameters
  of the HLK-LD2410 presence sensor.

  #define SERIAL_BAUD_RATE sets the serial monitor baud rate

  Communication with the sensor is handled by the 
  "MyLD2410" library Copyright (c) Iavor Veltchev 2024

  Use only hardware UART at the default baud rate 256000,
  or change the #define LD2410_BAUD_RATE to match your sensor.
  For ESP32 or other boards that allow dynamic UART pins,
  modify the RX_PIN and TX_PIN defines

  Connection diagram:
  Arduino/ESP32 RX  -- TX LD2410 
  Arduino/ESP32 TX  -- RX LD2410
  Arduino/ESP32 GND -- GND LD2410
  Provide sufficient power to the sensor Vcc (200mA, 5-12V) 
*/

#if defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_LEONARDO)
//ARDUINO_SAMD_NANO_33_IOT RX_PIN is D1, TX_PIN is D0
//ARDUINO_AVR_LEONARDO RX_PIN(RXI) is D0, TX_PIN(TXO) is D1
#define sensorSerial Serial1
#elif defined(ARDUINO_XIAO_ESP32C3) || defined(ARDUINO_XIAO_ESP32C6)
//RX_PIN is D7, TX_PIN is D6
#define sensorSerial Serial0
#elif defined(ESP32)
//Other ESP32 device - choose available GPIO pins
#define sensorSerial Serial1
#define RX_PIN 16
#define TX_PIN 17
#else
#error "This sketch only works on ESP32, Arduino Nano 33IoT, and Arduino Leonardo (Pro-Micro)"
#endif

// User defines
#define SERIAL_BAUD_RATE 115200

//Change the communication baud rate here, if necessary
//#define LD2410_BAUD_RATE 256000
#include "MyLD2410.h"

MyLD2410 sensor(sensorSerial);
bool restore = true;
byte maxMovingGate, maxStationaryGate, noOneWindow;
bool fineRes;
MyLD2410::ValuesArray movingParameters, stationaryParameters;

void printValue(const byte &val) {
  Serial.print(' ');
  Serial.print(val);
}

void printParameters(bool first = false) {
  if (first) {
    Serial.print("Firmware: ");
    Serial.println(sensor.getFirmware());
    Serial.print("Protocol version: ");
    Serial.println(sensor.getVersion());
    Serial.print("Bluetooth MAC address: ");
    Serial.println(sensor.getMACstr());
  }
  const MyLD2410::ValuesArray &mThr = sensor.getMovingThresholds();
  const MyLD2410::ValuesArray &sThr = sensor.getStationaryThresholds();

  Serial.print("Resolution (gate-width): ");
  Serial.print(sensor.getResolution());
  Serial.print("cm\nMax range: ");
  Serial.print(sensor.getRange_cm());
  Serial.print("cm\nMoving thresholds    [0,");
  Serial.print(mThr.N);
  Serial.print("]:");
  mThr.forEach(printValue);
  Serial.print("\nStationary thresholds[0,");
  Serial.print(sThr.N);
  Serial.print("]:");
  sThr.forEach(printValue);
  Serial.print("\nNo-one window: ");
  Serial.print(sensor.getNoOneWindow());
  Serial.println('s');
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

  Serial.println("Current set of parameters");
  printParameters(true);
  fineRes = (sensor.getResolution() == 20);
  noOneWindow = sensor.getNoOneWindow();
  movingParameters = sensor.getMovingThresholds();
  stationaryParameters = sensor.getStationaryThresholds();

  delay(2000);
  //Set the no-one window parameter to 3 seconds
  Serial.println("\nSetting the no-one window to 3 seconds");
  if (sensor.setNoOneWindow(3)) printParameters();
  else {
    Serial.println("Fail");
    return;
  }

  delay(2000);
  //Change the resolution fine->coarse or coarse->fine
  Serial.print("\nSetting the resolution (gate-width) to ");
  Serial.print(((fineRes) ? 75 : 20));
  Serial.println("cm");
  if (sensor.setResolution(!fineRes)) printParameters();
  else {
    Serial.println("Fail");
    return;
  }

  delay(2000);
  //Set the maximum moving gate parameter to 4
  Serial.println("\nSetting the maximum moving gate parameter to 4");
  if (sensor.setMaxMovingGate(4)) printParameters();
  else {
    Serial.println("Fail");
    return;
  }

  delay(2000);
  //Set the maximum stationary gate parameter to 5
  Serial.println("\nSetting the maximum stationary gate parameter to 5");
  if (sensor.setMaxStationaryGate(5)) printParameters();
  else {
    Serial.println("Fail");
    return;
  }

  delay(2000);
  //Set the thresholds of gate 3 to 35 (moving) and 45 (stationary)
  Serial.println("\nSetting the thresholds of gate 3 to 35 (moving) and 45 (stationary)");
  if (sensor.setGateParameters(3, 35, 45)) printParameters();
  else {
    Serial.println("Fail");
    return;
  }

  Serial.println("\nSuccess!");
}

void loop() {
  if (restore) {
    restore = false;
    Serial.println("\nRestoring the original settings");
    delay(2000);
    if (sensor.setResolution(fineRes) && sensor.setGateParameters(movingParameters, stationaryParameters, noOneWindow)) {
      printParameters();
      Serial.println("Done!");
    } else {
      Serial.println("\nO-oh, something went wrong...");
      Serial.println("\nLoad the \"factory_reset\" sketch to restore the defaults...");
    }
  }
}
