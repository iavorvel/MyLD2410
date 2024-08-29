/*
  This program sets/resets the Bluetooth password of the HLK-LD2410
  presence sensor.

  To set the BT password to "passwd", run the sketch as is
  To reset the BT password to "HiLink", uncomment line 43 and run the sketch
  (line 43) #define RESET_PASSWORD

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
// #define DEBUG_MODE
// #define RESET_PASSWORD
#define SERIAL_BAUD_RATE 115200

// Change the communication baud rate here, if necessary
// #define LD2410_BAUD_RATE 256000
#include "MyLD2410.h"

#ifdef DEBUG_MODE
MyLD2410 sensor(sensorSerial, true);
#else
MyLD2410 sensor(sensorSerial);
#endif

#ifndef RESET_PASSWORD
String NewPassword("passwd");
#endif

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

  Serial.println("Established communication with the sensor.");

#ifdef RESET_PASSWORD
  Serial.println("Setting the Bluetooth password to: \"HiLink\"");
  if (sensor.resetBTpassword())
    Serial.println(" + Success!");
  else
    Serial.println(" - Fail!");
#else
  Serial.print("Setting the Bluetooth password to: \"");
  Serial.print(NewPassword);
  Serial.println('\"');
  if (sensor.setBTpassword(NewPassword))
    Serial.println(" + Success!");
  else
    Serial.println(" - Fail!");
#endif
}

void loop() {
}