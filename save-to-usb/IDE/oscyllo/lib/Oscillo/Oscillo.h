// Oscillo.h
#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef Oscillo_h
#define Oscillo_h
// ======================
extern "C" { uint16 system_adc_read(void); }

#define GRAPH_LEN   256
#define JSON_LEN    2048

#define MILLIVOLTS_PER_STEP    16.329
// ======================

class Oscillo
{
private:
    unsigned long LastDuration = 0;

public:

    uint16_t Data[GRAPH_LEN];
    size_t Length();

    Oscillo();
    ~Oscillo();
    void DoRead();
    size_t Serialize(char* buffer, size_t bufferSize);

    uint16_t Average();
    uint16_t Max();
    
    unsigned long Duration();
    unsigned long SampleRate();
    unsigned long UnwrapFreq();
};

float Voltage(uint16_t value);

// ======================
#endif