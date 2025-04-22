// Transfer.h
#include <Arduino.h>

#ifndef Transfer_h
#define Transfer_h
// ======================

// Classification of serial bus transfer baud speed

// Debug baud rates

#define DEBUG_BAUD_ONE 1200
#define DEBUG_BAUD_TWO 2400
#define DEBUG_BAUD_THREE 4800

// Standard baud rates

#define STANDART_BAUD_ONE 9600
#define STANDART_BAUD_TWO 35000
#define STANDART_BAUD_THREE 115200

// High-speed baud rates

#define HIGHT_BAUD_ONE 230400
#define HIGHT_BAUD_TWO  460800
#define HIGHT_BAUD_THREE 921600

// Experimental/unstable baud rates
// Useless settings...

#define UNSTABLE_BAUD_ONE 1000000
#define UNSTABLE_BAUD_TWO 2000000

// ======================

extern unsigned long PROGMEM BaudArray[6];

// ======================

void StartSerial();
void NextBaud();
unsigned long SerialSpeed();

size_t SerialSend(const char* data);
unsigned long SerialSentBytes();

#endif