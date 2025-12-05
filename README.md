# MyLD2410 Arduino Library
## Introduction

This library covers the complete set of serial commands for the LD2410 presence sensor, including HLK-LD2410B and  HLK-LD2410C. It has no external dependencies and will work on any Arduino and ESP32 board. 

The latest firmware version 2.44 is supported, providing auxiliary illumination-based controls and automatic thresholds detection.

HLK-LD2410C<br>
![LD2410C](images/ld2410c.png)

HLK-LD2410B<br>
![LD2410B](images/ld2410.png)

## Installation

1. It is recommended to install this library from the Arduino Library Manager (search for "MyLD2410"). However, if the Arduino Library Manager does not find it, or you prefer to install it yourself, download or clone this repository.

    - Unzip the downloaded archive in the Arduino/libraries folder. 

    - Restart the Arduino IDE.

1. Include the header file: `#include <MyLD2410.h>` in your sketch.

## Documentation

[MyLD2410 Documentation](https://iavorvel.github.io/site/MyLD2410/classMyLD2410.html)


## Usage
* Decide which **hardware** serial stream you'll be using for communication. For 5V Arduino boards you must use a voltage divider (1:2 or 2:3) on the TX line to protect the RX pin of the sensor, which operates at 3.3V.  

`#define sensorSerial Serial1`


|  Board | Serial | RX pin | Tx pin |
|--------|--------|:--:|:--:|
| ESP32 WROOM-32 | Serial1 | 16 | 17 |
| ESP32-S3 WROOM | Serial1 | 18 | 17 |
| Xiao-ESP32-c3 | Serial0 | D7 | D6 |
| Xiao-ESP32-c6 | Serial0 | D7 | D6 |
| Adruino Nano 33 IoT | Serial1 | D1 | D0 |
| Adruino Pro-Micro (Leonardo, **5V!**) | Serial1 | D0 | D1 |

* Create a global instance of the sensor object

```c++
MyLD2410 sensor(sensorSerial);
```

* In the `setup()` function, begin serial communication with baud rate `LD2420_BAUD_RATE` (256000). On ESP32-WROOM: RX_PIN(16), TX_PIN(17). Check the exact pin numbers for your board. Then call `sensor.begin()` to begin communication with the sensor.

```c++
sensorSerial.begin(LD2410_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);

if (!sensor.begin()) {
Serial.println("Failed to communicate with the sensor");
while (true);
}
```

* In the `loop()` function, call as often as possible `sensor.check()`. This function returns:
    
    1. `MyLD2410::DATA` if a data frame is received
    
    1. `MyLD2410::ACK` if a command-response frame is received
    
    1. `MyLD2410::FAIL` if no useful information was processed

* Use the many convenience functions to extract/modify the sensor data (see the examples below).

* There is a useful **Debug** feature that prints all frames received from the sensor. To enable debugging you instantiate the sensor with a second argument set to true:

    `MyLD2410 sensor(sensorSerial, true);`

    Debugging can be enabled and disabled at runtime with `sensor.debugOn();` and  `sensor.debugOff();`

## Examples
* Once the library is installed, navigate to: `File`&rarr;`Examples`&rarr;`MyLD2410` to play with the examples. They are automatically configured for some popular boards (see the table above). For other boards, minor (trivial) modifications may be necessary. Study the `board_select.h` header file in each example.
    
    1. `sensor_data` - retrieves all data frames from the sensor and outputs useful information every second. Handles both basic and enhanced (engineering) modes.

        Sample output:
        ```text
        Both moving and stationary, distance: 131cm
        MOVING    = 100@30cm 
        signals->[ 100 100 31 36 34 30 10 3 6 ] thresholds:[ 50 50 40 30 20 15 15 15 15 ]
        STATIONARY= 100@263cm 
        signals->[ 0 0 82 100 100 79 79 71 35 ] thresholds:[ 0 0 40 40 30 30 20 20 20 ]
        Light level: 175
        Output level: HIGH
        ```

        ```text
        No target
        Light level: 163
        Output level: LOW
        ```
    
    1. `print_parameters`- prints the current device parameters. Note your firmware version and upgrade it using the Bluetooth app to get the latest features.

        Sample output:
        ```text
        Firmware: 2.44.24073110
        Protocol version: 1
        Bluetooth MAC address: 49:AF:7B:68:81:28
        Resolution (gate-width): 75cm
        Max range: 675cm
        Moving thresholds    [0,8]: 50 50 40 30 20 15 15 15 15
        Stationary thresholds[0,8]: 0 0 40 40 30 30 20 20 20
        No-one window: 5s
        Auxiliary Configuration: no light control
        Default output level: LOW
        Done!
        ```

    1. `factory_reset` - restores the factory default values of the sensor parameters.

    1. `modify_parameters` - demonstrates how to modify various sensor parameters. At the end, the original state of the sensor is restored.

    1. `auto_thresholds` - automatically detect the thresholds for the particular environment. This feature was introduced in firmware **version 2.44**. 

    1. `set_bt_password` - sets a new Bluetooth password, or resets the password to its default value "HiLink". _Be careful not to get locked out of your sensor._ 
    
        - The password must be 6 characters long. If you enter a longer password (on line 54), it will be truncated to 6 characters. 
        
        - If you provide a shorter one, it will be padded with spaces (0x20). 
        
        - To restore the default password, uncomment the line `#define RESET_PASSWORD` and flash the sketch again.

    1. `set_baud_rate` - sets and tests a new baud rate for communication with the sensor. _Be careful not to get locked out of your sensor._
        
        - This is useful when you intend to use the LD2410 sensor on a board like Arduino Nano/Uno. First lower the baud rate to 38400 and then use `SoftwareSerial` to communicate on pins 10 (RX) and 11 (TX). Check out the `board_select.h` header file in `sensor_data`.

## Have fun!
