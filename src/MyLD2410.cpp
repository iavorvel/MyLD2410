#include "MyLD2410.h"

/*** BEGIN LD2410 namespace ***/
namespace LD2410
{
  const char *tStatus[7]{
      "No target",
      "Moving only",
      "Stationary only",
      "Both moving and stationary",
      "Auto thresholds in progress",
      "Auto thresholds successful",
      "Auto thresholds failed"};
  const byte headData[4]{0xF4, 0xF3, 0xF2, 0xF1};
  const byte tailData[4]{0xF8, 0xF7, 0xF6, 0xF5};
  const byte headConfig[4]{0xFD, 0xFC, 0xFB, 0xFA};
  const byte tailConfig[4]{4, 3, 2, 1};
  const byte configEnable[6]{4, 0, 0xFF, 0, 1, 0};
  const byte configDisable[4]{2, 0, 0xFE, 0};
  const byte MAC[6]{4, 0, 0xA5, 0, 1, 0};
  const byte firmware[4]{2, 0, 0xA0, 0};
  const byte res[4]{2, 0, 0xAB, 0};
  const byte resCoarse[6]{4, 0, 0xAA, 0, 0, 0};
  const byte resFine[6]{4, 0, 0xAA, 0, 1, 0};
  const byte changeBaud[6]{4, 0, 0xA1, 0, 7, 0};
  const byte reset[4]{2, 0, 0xA2, 0};
  const byte reboot[4]{2, 0, 0xA3, 0};
  const byte BTon[6]{4, 0, 0xA4, 0, 1, 0};
  const byte BToff[6]{4, 0, 0xA4, 0, 0, 0};
  const byte BTpasswd[10]{8, 0, 0xA9, 0, 0x48, 0x69, 0x4C, 0x69, 0x6E, 0x6B};
  const byte param[4]{2, 0, 0x61, 0};
  const byte engOn[4]{2, 0, 0x62, 0};
  const byte engOff[4]{2, 0, 0x63, 0};
  const byte auxQuery[4]{2, 0, 0xAE, 0};
  const byte auxConfig[8]{6, 0, 0xAD, 0, 0, 0x80, 0, 0};
  const byte autoBegin[6]{4, 0, 0x0B, 0, 0x0A, 0};
  const byte autoQuery[4]{2, 0, 0x1B, 0};
  byte gateParam[0x16]{0x14, 0, 0x64, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0};
  byte maxGate[0x16]{0x14, 0, 0x60, 0, 0, 0, 8, 0, 0, 0, 1, 0, 8, 0, 0, 0, 2, 0, 5, 0, 0, 0};

  String byte2hex(byte b, bool addZero = true)
  {
    String bStr(b, HEX);
    bStr.toUpperCase();
    if (addZero && (bStr.length() == 1))
      return "0" + bStr;
    return bStr;
  }
  void printBuf(const byte *buf, byte size)
  {
    for (byte i = 0; i < size; i++)
    {
      Serial.print(byte2hex(buf[i]));
      Serial.print(' ');
    }
    Serial.println();
    Serial.flush();
  }
  bool bufferEndsWith(const byte *buf, int iMax, const byte *other)
  {
    for (int j = 3; j >= 0; j--)
    {
      if (--iMax < 0)
        iMax = 3;
      if (buf[iMax] != other[j])
        return false;
    }
    return true;
  }
}
/*** END LD2410 namespace ***/

MyLD2410::Response MyLD2410::check()
{
  while (sensor->available())
  {
    headBuf[headBufI++] = byte(sensor->read());
    headBufI %= 4;
    if (LD2410::bufferEndsWith(headBuf, headBufI, LD2410::headConfig) && processAck())
      return ACK;
    if (LD2410::bufferEndsWith(headBuf, headBufI, LD2410::headData) && processData())
      return DATA;
  }
  return FAIL;
}

bool MyLD2410::sendCommand(const byte *command)
{
  byte size = command[0] + 2;
  // LD2410::printBuf(command, size);
  sensor->write(LD2410::headConfig, 4);
  sensor->write(command, size);
  sensor->write(LD2410::tailConfig, 4);
  sensor->flush();
  unsigned long giveUp = millis() + 2000UL;
  while (millis() < giveUp)
  {
    while (sensor->available())
    {
      headBuf[headBufI++] = byte(sensor->read());
      headBufI %= 4;
      if (LD2410::bufferEndsWith(headBuf, headBufI, LD2410::headConfig))
        return processAck();
    }
  }
  return false;
}

bool MyLD2410::readFrame()
{
  int frameSize = -1, bytes = 2;
  inBufI = 0;
  unsigned long frameTimeout = millis() + 100;
  while (bytes) // read two bytes for frame size
  {

    if (sensor->available())
    {
      inBuf[inBufI++] = byte(sensor->read());
      bytes--;
    }
    else if (millis() > frameTimeout)
      return false; // timeout
  }

  frameSize = (inBuf[0]) | (inBuf[1] << 8);

  if (frameSize <= 0)
    return false; // Corrupted frame received
  frameSize += 4;
  if (frameSize > LD2410_BUFFER_SIZE)
    return false; // Frame too large, discard

  inBufI = 0;
  frameTimeout = millis() + 100;
  while (frameSize > 0) // read the rest of the frame
  {
    if (sensor->available())
    {
      inBuf[inBufI++] = byte(sensor->read());
      frameSize--;
    }
    else if (millis() > frameTimeout)
      return false; // timeout
  }
  return true;
}

bool MyLD2410::processAck()
{
  if (!readFrame())
    return false;
  if (_debug)
    LD2410::printBuf(inBuf, inBufI);
  if (!LD2410::bufferEndsWith(inBuf, inBufI, LD2410::tailConfig))
    return false;
  unsigned long command = inBuf[0] | (inBuf[1] << 8);
  if (inBuf[2] | (inBuf[3] << 8))
    return false;
  switch (command)
  {
  case 0x1FF: // entered config mode
    isConfig = true;
    version = inBuf[4] | (inBuf[5] << 8);
    bufferSize = inBuf[6] | (inBuf[7] << 8);
    break;
  case 0x1FE: // exited config mode
    isConfig = false;
    break;
  case 0x1A5: // MAC
    for (int i = 0; i < 6; i++)
      MAC[i] = inBuf[i + 4];
    MACstr = LD2410::byte2hex(MAC[0]);
    for (int i = 1; i < 6; i++)
      MACstr += ":" + LD2410::byte2hex(MAC[i]);
    break;
  case 0x1A0: // Firmware
    firmware = LD2410::byte2hex(inBuf[7], false);
    firmware += "." + LD2410::byte2hex(inBuf[6]);
    firmware += "." + LD2410::byte2hex(inBuf[11]);
    firmware += LD2410::byte2hex(inBuf[10]);
    firmware += LD2410::byte2hex(inBuf[9]);
    firmware += LD2410::byte2hex(inBuf[8]);
    firmwareMajor = inBuf[7];
    firmwareMinor = inBuf[6];
    break;
  case 0x1AB: // Query Resolution
    fineRes = (inBuf[4]);
    break;
  case 0x1AE: // Query auxiliary control parameters
    lightControl = LightControl(inBuf[4]);
    lightThreshold = inBuf[5];
    outputControl = OutputControl(inBuf[6]);
    break;
  case 0x11B:
    autoStatus = AutoStatus(inBuf[4]);
    break;
  case 0x1A3: // Reboot
    isEnhanced = false;
    isConfig = false;
    break;
  case 0x161: // Query parameters
    maxRange = inBuf[5];
    movingThresholds.setN(inBuf[6]);
    stationaryThresholds.setN(inBuf[7]);
    for (byte i = 0; i <= movingThresholds.N; i++)
      movingThresholds.values[i] = inBuf[8 + i];
    for (byte i = 0; i <= stationaryThresholds.N; i++)
      stationaryThresholds.values[i] = inBuf[17 + i];
    noOne_window = inBuf[26] | (inBuf[27] << 8);
    break;
  case 0x162:
    isEnhanced = true;
    break;
  case 0x163:
    isEnhanced = false;
    break;
  case 0x164:
    if (LD2410::gateParam[7] == 0xFF)
      LD2410::gateParam[7] = 0;
  }
  return (true);
}

bool MyLD2410::processData()
{
  if (!readFrame())
    return false;
  unsigned long now = millis();
  if (_debug)
  {
    Serial.print('#');
    Serial.print(dataFrames + 1);
    Serial.print(" [");
    Serial.print(now);
    Serial.print("ms] : ");
    LD2410::printBuf(inBuf, inBufI);
  }
  if (!LD2410::bufferEndsWith(inBuf, inBufI, LD2410::tailData))
    return false;
  if (((inBuf[0] == 1) || (inBuf[0] == 2)) && (inBuf[1] == 0xAA))
  { // Basic mode and Enhanced
    ++dataFrames;
    sData.timestamp = now;
    sData.status = inBuf[2] & 7;
    sData.mTargetDistance = inBuf[3] | (inBuf[4] << 8);
    sData.mTargetSignal = inBuf[5];
    sData.sTargetDistance = inBuf[6] | (inBuf[7] << 8);
    sData.sTargetSignal = inBuf[8];
    sData.distance = inBuf[9] | (inBuf[10] << 8);
    if (inBuf[0] == 1)
    { // Enhanced mode only
      isEnhanced = true;
      sData.mTargetSignals.setN(inBuf[11]);
      sData.sTargetSignals.setN(inBuf[12]);
      byte *p = inBuf + 13;
      for (byte i = 0; i <= sData.mTargetSignals.N; i++)
        sData.mTargetSignals.values[i] = *(p++);
      for (byte i = 0; i <= sData.sTargetSignals.N; i++)
        sData.sTargetSignals.values[i] = *(p++);
      lightLevel = *(p++);
      outLevel = *p;
    }
    else
    { // Basic mode only
      isEnhanced = false;
      sData.mTargetSignals.setN(0);
      sData.sTargetSignals.setN(0);
      lightLevel = 0;
      outLevel = 0;
    }
  }
  else
    return false;
  return true;
}

/**
@brief Construct from a serial stream object
*/
MyLD2410::MyLD2410(Stream &serial, bool debug)
{
  sensor = &serial;
  _debug = debug;
}

bool MyLD2410::begin()
{
  // Wait for the sensor to come online, or to timeout.
  unsigned long giveUp = millis() + 2000UL;
  bool online = false;
  isConfig = false;
  sendCommand(LD2410::configDisable);
  while (millis() < giveUp)
  {
    if (check())
    {
      online = true;
      break;
    }
    delay(110);
  }
  return online;
}

void MyLD2410::end()
{
  isConfig = false;
  isEnhanced = false;
}

void MyLD2410::debugOn()
{
  _debug = true;
}

void MyLD2410::debugOff()
{
  _debug = false;
}

bool MyLD2410::inConfigMode()
{
  return isConfig;
}

bool MyLD2410::inBasicMode()
{
  return !isEnhanced;
}

bool MyLD2410::inEnhancedMode()
{
  return isEnhanced;
}

byte MyLD2410::getStatus()
{
  return (isDataValid()) ? sData.status : 0xFF;
}

const char *MyLD2410::statusString()
{
  return LD2410::tStatus[sData.status];
}

unsigned long MyLD2410::getTimestamp()
{
  return sData.timestamp;
}

unsigned long MyLD2410::getFrameCount()
{
  return dataFrames;
}

bool MyLD2410::isDataValid()
{
  return (millis() < sData.timestamp + 500UL);
}

bool MyLD2410::presenceDetected()
{
  return isDataValid() && (sData.status) && (sData.status < 4); // 1,2,3
}

bool MyLD2410::stationaryTargetDetected()
{
  return isDataValid() && ((sData.status == 2) || (sData.status == 3)); // 2,3
}

unsigned long MyLD2410::stationaryTargetDistance()
{
  return sData.sTargetDistance;
}

byte MyLD2410::stationaryTargetSignal()
{
  return sData.sTargetSignal;
}

const MyLD2410::ValuesArray &MyLD2410::getStationarySignals()
{
  return sData.sTargetSignals;
}

bool MyLD2410::movingTargetDetected()
{
  return isDataValid() && ((sData.status == 1) || (sData.status == 3)); // 1,3
}

unsigned long MyLD2410::movingTargetDistance()
{
  return sData.mTargetDistance;
}

byte MyLD2410::movingTargetSignal()
{
  return sData.mTargetSignal;
}

const MyLD2410::ValuesArray &MyLD2410::getMovingSignals()
{
  return sData.mTargetSignals;
}

unsigned long MyLD2410::detectedDistance()
{
  return sData.distance;
}

const byte *MyLD2410::getMAC()
{
  if (MACstr.length() == 0)
    requestMAC();
  return MAC;
}

String MyLD2410::getMACstr()
{
  if (MACstr.length() == 0)
    requestMAC();
  return MACstr;
}

String MyLD2410::getFirmware()
{
  if (firmware.length() == 0)
    requestFirmware();
  return firmware;
}

byte MyLD2410::getFirmwareMajor()
{
  if (!firmwareMajor)
    requestFirmware();
  return firmwareMajor;
}

byte MyLD2410::getFirmwareMinor()
{
  if (!firmwareMajor)
    requestFirmware();
  return firmwareMinor;
}

unsigned long MyLD2410::getVersion()
{
  if (version == 0)
  {
    configMode();
    configMode(false);
  }
  return version;
}

const MyLD2410::SensorData &MyLD2410::getSensorData()
{
  return sData;
}

const MyLD2410::ValuesArray &MyLD2410::getMovingThresholds()
{
  if (!maxRange)
    requestParameters();
  return movingThresholds;
}

const MyLD2410::ValuesArray &MyLD2410::getStationaryThresholds()
{
  if (!maxRange)
    requestParameters();
  return stationaryThresholds;
}

byte MyLD2410::getRange()
{
  if (!maxRange)
    requestParameters();
  return maxRange;
}

unsigned long MyLD2410::getRange_cm()
{
  return (getRange() + 1) * getResolution();
}

byte MyLD2410::getNoOneWindow()
{
  if (!maxRange)
    requestParameters();
  return noOne_window;
}

bool MyLD2410::configMode(bool enable)
{
  if (enable && !isConfig)
    return sendCommand(LD2410::configEnable);
  if (!enable && isConfig)
    return sendCommand(LD2410::configDisable);
  return false;
}

bool MyLD2410::enhancedMode(bool enable)
{
  if (isConfig)
    return sendCommand(((enable) ? LD2410::engOn : LD2410::engOff));
  else
    return configMode() && sendCommand(((enable) ? LD2410::engOn : LD2410::engOff)) && configMode(false);
}

bool MyLD2410::requestAuxConfig()
{
  if (isConfig)
    return sendCommand(LD2410::auxQuery);
  return configMode() && sendCommand(LD2410::auxQuery) && configMode(false);
}

bool MyLD2410::autoThresholds(byte _timeout)
{
  byte cmd[6];
  memcpy(cmd, LD2410::autoBegin, 6);
  if (_timeout)
    cmd[4] = _timeout;
  if (isConfig)
    return sendCommand(cmd);
  return configMode() && sendCommand(cmd) && configMode(false);
}

AutoStatus MyLD2410::getAutoStatus()
{
  bool res = false;
  if (isConfig)
    res = sendCommand(LD2410::autoQuery);
  else
    res = configMode() && sendCommand(LD2410::autoQuery) && configMode(false);
  if (res)
    return autoStatus;
  return AutoStatus::NOT_SET;
}

bool MyLD2410::requestMAC()
{
  if (isConfig)
    return sendCommand(LD2410::MAC);
  return configMode() && sendCommand(LD2410::MAC) && configMode(false);
}

bool MyLD2410::requestFirmware()
{
  if (isConfig)
    return sendCommand(LD2410::firmware);
  return configMode() && sendCommand(LD2410::firmware) && configMode(false);
}

bool MyLD2410::requestResolution()
{
  if (isConfig)
    return sendCommand(LD2410::res);
  return configMode() && sendCommand(LD2410::res) && configMode(false);
}

bool MyLD2410::setResolution(bool fine)
{
  if (isConfig && sendCommand(((fine) ? LD2410::resFine : LD2410::resCoarse)))
    return sendCommand(LD2410::res);
  return configMode() && sendCommand(((fine) ? LD2410::resFine : LD2410::resCoarse)) && sendCommand(LD2410::res) && configMode(false);
}

bool MyLD2410::requestParameters()
{
  if (isConfig)
    return sendCommand(LD2410::param);
  return configMode() && sendCommand(LD2410::param) && configMode(false);
}

bool MyLD2410::setGateParameters(byte gate, byte movingThreshold, byte stationaryThreshold)
{
  if (movingThreshold > 100)
    movingThreshold = 100;
  if (stationaryThreshold > 100)
    stationaryThreshold = 100;
  byte *cmd = LD2410::gateParam;
  if (gate > 8)
  {
    cmd[6] = 0xFF;
    cmd[7] = 0xFF;
  }
  else
  {
    cmd[6] = gate;
    cmd[7] = 0;
  }
  cmd[12] = movingThreshold;
  cmd[18] = stationaryThreshold;
  if (isConfig)
    return sendCommand(cmd) && sendCommand(LD2410::param);
  return configMode() && sendCommand(cmd) && sendCommand(LD2410::param) && configMode(false);
}

bool MyLD2410::setMovingThreshold(byte gate, byte movingThreshold)
{
  if (gate > 8)
    return false;
  if (!stationaryThresholds.N)
    requestParameters();
  return setGateParameters(gate, movingThreshold, stationaryThresholds.values[gate]);
}

bool MyLD2410::setStationaryThreshold(byte gate, byte stationaryThreshold)
{
  if (gate > 8)
    return false;
  if (!movingThresholds.N)
    requestParameters();
  return setGateParameters(gate, movingThresholds.values[gate], stationaryThreshold);
}

bool MyLD2410::setMaxGate(byte movingGate, byte staticGate, byte noOneWindow)
{
  if (movingGate > 8)
    movingGate = 8;
  if (staticGate > 8)
    staticGate = 8;
  byte *cmd = LD2410::maxGate;
  cmd[6] = movingGate;
  cmd[12] = staticGate;
  cmd[18] = noOneWindow;
  if (isConfig && sendCommand(cmd))
    return sendCommand(LD2410::param);
  return configMode() && sendCommand(cmd) && sendCommand(LD2410::param) && configMode(false);
}

bool MyLD2410::setGateParameters(
    const ValuesArray &moving_thresholds,
    const ValuesArray &stationary_thresholds,
    byte noOneWindow)
{
  if (!isConfig)
    configMode();
  bool success = isConfig;
  if (success)
  {
    for (byte i = 0; i < 9; i++)
    {
      if (!setGateParameters(i, moving_thresholds.values[i], stationary_thresholds.values[i]))
      {
        success = false;
        break;
      }
      delay(20);
    }
  }
  return success && setMaxGate(moving_thresholds.N, stationary_thresholds.N, noOneWindow) && configMode(false);
}

bool MyLD2410::setNoOneWindow(byte noOneWindow)
{
  if (!maxRange)
    requestParameters();
  if (noOne_window == noOneWindow)
    return true;
  return setMaxGate(movingThresholds.N, stationaryThresholds.N, noOneWindow);
}

bool MyLD2410::setMaxMovingGate(byte movingGate)
{
  if (!maxRange)
    requestParameters();
  if (movingThresholds.N == movingGate)
    return true;
  if (!noOne_window)
    noOne_window = 5;
  if (movingGate > 8)
    movingGate = 8;
  return setMaxGate(movingGate, stationaryThresholds.N, noOne_window);
}

bool MyLD2410::setMaxStationaryGate(byte stationaryGate)
{
  if (!maxRange)
    requestParameters();
  if (stationaryThresholds.N == stationaryGate)
    return true;
  if (!noOne_window)
    noOne_window = 5;
  if (stationaryGate > 8)
    stationaryGate = 8;
  return setMaxGate(movingThresholds.N, stationaryGate, noOne_window);
}

byte MyLD2410::getMaxMovingGate()
{
  if (!movingThresholds.N)
    requestParameters();
  return movingThresholds.N;
}

byte MyLD2410::getMaxStationaryGate()
{
  if (!stationaryThresholds.N)
    requestParameters();
  return stationaryThresholds.N;
}

bool MyLD2410::requestReset()
{
  if (isConfig)
    return sendCommand(LD2410::reset) && sendCommand(LD2410::param) && sendCommand(LD2410::res);
  return configMode() && sendCommand(LD2410::reset) && sendCommand(LD2410::param) && sendCommand(LD2410::res) && configMode(false);
}

bool MyLD2410::requestReboot()
{
  if (isConfig)
    return sendCommand(LD2410::reboot);
  return configMode() && sendCommand(LD2410::reboot);
}

bool MyLD2410::requestBTon()
{
  if (isConfig)
    return sendCommand(LD2410::BTon);
  return configMode() && sendCommand(LD2410::BTon) && configMode(false);
}

bool MyLD2410::requestBToff()
{
  if (isConfig)
    return sendCommand(LD2410::BToff);
  return configMode() && sendCommand(LD2410::BToff) && configMode(false);
}

bool MyLD2410::setBTpassword(const char *passwd)
{
  byte cmd[10];
  for (unsigned int i = 0; i < 4; i++)
    cmd[i] = LD2410::BTpasswd[i];

  for (unsigned int i = 0; i < 6; i++)
  {
    if (i < strlen(passwd))
      cmd[4 + i] = byte(passwd[i]);
    else
      cmd[4 + i] = byte(' ');
  }
  if (isConfig)
    return sendCommand(cmd);
  return configMode() && sendCommand(cmd) && configMode(false);
}

bool MyLD2410::setBTpassword(const String &passwd)
{
  return setBTpassword(passwd.c_str());
}

bool MyLD2410::resetBTpassword()
{
  if (isConfig)
    return sendCommand(LD2410::BTpasswd);
  return configMode() && sendCommand(LD2410::BTpasswd) && configMode(false);
}

bool MyLD2410::setBaud(byte baud)
{
  if ((baud < 1) || (baud > 8))
    return false;
  byte cmd[6];
  memcpy(cmd, LD2410::changeBaud, 6);
  cmd[4] = baud;
  if (isConfig)
    return sendCommand(cmd) && requestReboot();
  return configMode() && sendCommand(cmd) && requestReboot();
}

byte MyLD2410::getResolution()
{
  if (fineRes >= 0)
    return ((fineRes == 1) ? 20 : 75);
  if (isConfig)
  {
    if (sendCommand(LD2410::res))
      return getResolution();
  }
  else
  {
    if (configMode() && sendCommand(LD2410::res) && configMode(false))
      return getResolution();
  }
  return 0;
}

byte MyLD2410::getLightLevel()
{
  return lightLevel;
}

LightControl MyLD2410::getLightControl()
{
  if (lightControl == LightControl::NOT_SET)
    requestAuxConfig();
  return lightControl;
}

byte MyLD2410::getLightThreshold()
{
  if (lightControl == LightControl::NOT_SET)
    requestAuxConfig();
  return lightThreshold;
}

OutputControl MyLD2410::getOutputControl()
{
  if (outputControl == OutputControl::NOT_SET)
    requestAuxConfig();
  return outputControl;
}

bool MyLD2410::setAuxControl(
    LightControl light_control,
    byte light_threshold,
    OutputControl output_control)
{
  byte cmd[8];
  memcpy(cmd, LD2410::auxConfig, 8);
  cmd[4] = byte(light_control);
  cmd[5] = light_threshold;
  cmd[6] = byte(output_control);
  if (isConfig)
    return sendCommand(cmd) && requestAuxConfig();
  return configMode() && sendCommand(cmd) && requestAuxConfig() && configMode(false);
}

bool MyLD2410::resetAuxControl()
{
  if (isConfig)
    return sendCommand(LD2410::auxConfig) && requestAuxConfig();
  return configMode() && sendCommand(LD2410::auxConfig) && requestAuxConfig() && configMode(false);
}

byte MyLD2410::getOutLevel()
{
  return outLevel;
}
