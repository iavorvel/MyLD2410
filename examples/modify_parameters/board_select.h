#if defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_LEONARDO)
// ARDUINO_SAMD_NANO_33_IOT RX_PIN is D1, TX_PIN is D0
// ARDUINO_AVR_LEONARDO RX_PIN(RXI) is D0, TX_PIN(TXO) is D1
#define sensorSerial Serial1
#elif defined(ARDUINO_XIAO_ESP32C3) || defined(ARDUINO_XIAO_ESP32C6)
// RX_PIN is D7, TX_PIN is D6
#define sensorSerial Serial0
#elif defined(ESP32)
// Other ESP32 device - choose available GPIO pins
#define sensorSerial Serial1
#if defined(ARDUINO_ESP32S3_DEV)
#define RX_PIN 18
#define TX_PIN 17
#else
#define RX_PIN 16
#define TX_PIN 17
#endif
#else
#error "This sketch only works on ESP32, Arduino Nano 33IoT, and Arduino Leonardo (Pro-Micro)"
#endif
