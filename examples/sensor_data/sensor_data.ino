#ifdef ESP32
#define sensorSerial Serial2
#elif defined(ARDUINO_SAMD_NANO_33_IOT)
#define sensorSerial Serial1
#else
#error "This sketch only works on ESP32 or Arduino Nano 33IoT"
#endif

// User defines
#define SERIAL_BAUD_RATE 115200
#define ENHANCED_MODE
// #define DEBUG_MODE

#include <MyLD2410.h>

#ifdef DEBUG_MODE
MyLD2410 sensor(sensorSerial, true);
#else
MyLD2410 sensor(sensorSerial);
#endif

unsigned long nextPrint = 0, printEvery = 1000; // print every second

void printValue(const byte &val)
{
  Serial.print(' ');
  Serial.print(val);
}

void printData()
{
  Serial.print(sensor.statusString());
  if (sensor.presenceDetected())
  {
    Serial.print(", distance: ");
    Serial.print(sensor.detectedDistance());
    Serial.print("cm");
  }
  Serial.println();
  if (sensor.movingTargetDetected())
  {
    Serial.print(" MOVING    = ");
    Serial.print(sensor.movingTargetSignal());
    Serial.print("@");
    Serial.print(sensor.movingTargetDistance());
    Serial.print("cm ");
    if (sensor.inEnhancedMode())
    {
      Serial.print("\n signals->[");
      sensor.getMovingSignals().forEach(printValue);
      Serial.print(" ] thresholds:");
      sensor.getMovingThresholds().forEach(printValue);
    }
    Serial.println();
  }
  if (sensor.stationaryTargetDetected())
  {
    Serial.print(" STATIONARY= ");
    Serial.print(sensor.stationaryTargetSignal());
    Serial.print("@");
    Serial.print(sensor.stationaryTargetDistance());
    Serial.print("cm ");
    if (sensor.inEnhancedMode())
    {
      Serial.print("\n signals->[");
      sensor.getStationarySignals().forEach(printValue);
      Serial.print(" ] thresholds:");
      sensor.getStationaryThresholds().forEach(printValue);
    }
    Serial.println();
  }
  Serial.println();
}

void printParameters()
{
  const MyLD2410::ValuesArray &mThr = sensor.getMovingThresholds();
  const MyLD2410::ValuesArray &sThr = sensor.getStationaryThresholds();
  byte noone = sensor.getNoOneWindow(), range = sensor.getRange(), res = sensor.getResolution();

  Serial.print("Max range: ");
  Serial.print((range + 1) * res);
  Serial.print("cm\nMoving thresholds    [0,");
  Serial.print(mThr.N);
  Serial.print("]:");
  mThr.forEach(printValue);
  Serial.print("\nStationary thresholds[0,");
  Serial.print(sThr.N);
  Serial.print("]:");
  sThr.forEach(printValue);
  Serial.print("\nNo-one window: ");
  Serial.print(noone);
  Serial.println('s');
}

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
#ifdef ESP32
  sensorSerial.begin(LD2410_BAUD_RATE, SERIAL_8N1, 16, 17);
#else
  sensorSerial.begin(LD2410_BAUD_RATE);
#endif
  delay(2000);
  Serial.println(__FILE__);
  if (!sensor.begin())
  {
    Serial.println("Failed to communicate with the sensor");
    while (true)
      ;
  }
  printParameters();
#ifdef ENHANCED_MODE
  sensor.enhancedMode();
#endif
  delay(2000);
}

void loop()
{
  if ((sensor.check() == MyLD2410::Response::DATA) && (millis() > nextPrint))
  {
    nextPrint = millis() + printEvery;
    printData();
  }
}
