#include <Transfer.h>

// ==========================================

unsigned long PROGMEM BaudArray[6] = {
    STANDART_BAUD_ONE,
    STANDART_BAUD_TWO,
    STANDART_BAUD_THREE,

    HIGHT_BAUD_ONE,
    HIGHT_BAUD_TWO,
    HIGHT_BAUD_THREE
};

volatile uint8_t __baud_now = 2;
unsigned long __data_sent   = 0;

// ==========================================

void StartSerial() {
    Serial.begin(pgm_read_dword(&BaudArray[__baud_now]));
    delay(10);
}

void NextBaud() {
    __baud_now = (__baud_now + 1) % 6;
    Serial.updateBaudRate(pgm_read_dword(&BaudArray[__baud_now]));
    delay(10);
}

unsigned long SerialSpeed() {
    return pgm_read_dword(&BaudArray[__baud_now]);
}

unsigned long SerialSentBytes() {
    return __data_sent;
}

size_t SerialSend(const char* data) {
    size_t len = 0;
    len += Serial.write(data);
    len += Serial.write('\n');
    __data_sent += (unsigned long)len;
    return len;
}
