#ifndef MY_LD2410_H
#define MY_LD2410_H
#include <Arduino.h>
#define LD2410_BAUD_RATE 256000
#define LD2410_BUFFER_SIZE 0x40

class MyLD2410
{
public:
  enum Response
  {
    FAIL = 0,
    ACK,
    DATA
  };
  struct ValuesArray
  {
    byte values[9];
    byte N;
    void forEach(void (*func)(const byte &val)) const
    {
      for (byte i = 0; i < N; i++)
        func(values[i]);
    }
  };
  struct SensorData
  {
    byte status;
    unsigned long timestamp;
    unsigned long mTargetDistance;
    byte mTargetSignal;
    unsigned long sTargetDistance;
    byte sTargetSignal;
    unsigned long distance;
    // Enhanced data
    ValuesArray mTargetSignals;
    ValuesArray sTargetSignals;
  };

private:
  SensorData sData;
  ValuesArray stationaryThresholds;
  ValuesArray movingThresholds;
  byte maxRange = 0;
  byte noOne_window = 0;
  unsigned long version = 0;
  unsigned long bufferSize = 0;
  byte MAC[6];
  String MACstr = "";
  String firmware = "";
  int fineRes = -1; // -1=unknown; 0=75cm, 1=20cm
  bool isEnhanced = false;
  bool isConfig = false;
  unsigned long timeout = 2000;
  unsigned long dataLifespan = 500;
  byte inBuf[LD2410_BUFFER_SIZE];
  byte inBufI = 0;
  byte headBuf[4];
  byte headBufI = 0;
  Stream *sensor;
  bool _debug = false;
  bool isDataValid();
  bool readFrame();
  bool sendCommand(const byte *command);
  bool processAck();
  bool processData();

public:
  MyLD2410(Stream &serial, bool debug = false);

  // CONTROLS
  bool begin();
  void end();
  Response check();

  // GETTERS
  bool inConfigMode();
  bool inBasicMode();
  bool inEnhancedMode();
  bool presenceDetected();
  bool stationaryTargetDetected();
  const unsigned long stationaryTargetDistance();
  const byte stationaryTargetSignal();
  const ValuesArray &getStationarySignals();
  bool movingTargetDetected();
  const unsigned long movingTargetDistance();
  const byte movingTargetSignal();
  const ValuesArray &getMovingSignals();
  const unsigned long detectedDistance();

  const byte *getMAC();
  String getMACstr();
  String getFirmware();
  unsigned long getVersion();
  const SensorData &getSensorData();
  const char *statusString();
  byte getResolution();
  // parameters
  const ValuesArray &getMovingThresholds();
  const ValuesArray &getStationaryThresholds();
  byte getRange();
  byte getNoOneWindow();
  // end parameters

  // REQUESTS
  bool configMode(bool enable = true);
  bool enhancedMode(bool enable = true);
  bool requestMAC();
  bool requestFirmware();
  bool requestResolution();
  bool setResolution(bool fine = false);
  bool requestParameters();
  bool setGateParameters(byte gate, byte movingThreshold = 100, byte stationaryThreshold = 100);
  bool setMaxGate(byte movingGate, byte stationaryGate, byte noOneWindow = 5);
  bool requestReset();
  bool requestReboot();
  bool requestBTon();
  bool requestBToff();
};

#endif // MY_LD2410_H