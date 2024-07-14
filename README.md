# MyLD2410 Arduino library
## Introduction

This library covers the complete set of commands for the HLK-LD2410x presence sensor. It has no external dependencies and will work with any Arduino and ESP32 board.

![LD2410C](images/ld2410c.jpg)

## Installation

1. Download or clone the whole repository.
1. Unzip the downloaded archive in the Arduino/libraries folder. Restart the Arduino IDE.
1. Include the header file: `#include <my_ld2410.h>` in your sketch.

## Usage
* Decide which serial stream you'll be using for communication

`#define sensorSerial Serial2`

* Create a global instance of the sensor object

`MyLD2410 sensor(sensorSerial);`

* In the `setup()` function, begin serial communication with baud rate `LD2420_BAUD_RATE` (256000). On ESP32-WROOM: RX2(16), TX2(17). Check the exact pin numbers for your board. Then call `sensor.begin()` to begin communication with the sensor.

```
sensorSerial.begin(LD2410_BAUD_RATE, SERIAL_8N1, 16, 17);
if (!sensor.begin()) {
  Serial.println("Failed to communicate with the sensor");
  while (true);
}
```

* In the `loop()` function, call as often as possible `sensor.check()`. This function returns:
    1. `MyLD2410::DATA` if a data frame is received
    1. `MyLD2410::ACK` if a command-response frame is received
    1. `MyLD2410::FAIL` if no useful information was processed

* Use any of the many convenience functions to extract the data.

## Examples
* Once the library is installed, navigate to: `File->Examples->MyLD2401` to play with working examples.
