#include <Oscillo.h>

Oscillo::Oscillo() {
}

Oscillo::~Oscillo() {
    delete(Data);
}

void Oscillo::DoRead() {
    unsigned long startAt = micros();

    for (int i = 0; i < GRAPH_LEN; i++)
    {
        Data[i] = system_adc_read();
    }

    LastDuration = micros() - startAt;
}

unsigned long Oscillo::Duration() {
    return LastDuration;
}

size_t Oscillo::Length() {
    return GRAPH_LEN;
}

uint16_t Oscillo::Average() {
    unsigned long sum = 0;
    for (size_t i = 0; i < GRAPH_LEN; i++) {
        sum += Data[i];
    }
    return sum / GRAPH_LEN;
}

uint16_t Oscillo::Max() {
    uint16_t mx = 0;
    for (size_t i = 0; i < GRAPH_LEN; i++) {
        if (mx <= Data[i]) {
            mx = Data[i];
        }
    }
    return mx;
}

unsigned long Oscillo::UnwrapFreq() {
    return 1000000 / LastDuration;
}

unsigned long Oscillo::SampleRate() {
    return this->UnwrapFreq() * GRAPH_LEN;
}

// ----------

size_t Oscillo::Serialize(char* buffer, size_t bufferSize) {
    StaticJsonDocument<JSON_LEN-1> doc;

    doc["duration"]         = LastDuration;
    doc["length"]           = this->Length();
    doc["sample_rate"]      = this->SampleRate();
    doc["step_size"]        = MILLIVOLTS_PER_STEP;

    JsonArray arr = doc.createNestedArray("probes");
    for (size_t i = 0; i < GRAPH_LEN; i++) arr.add(Data[i]);

    return serializeJson(doc, buffer, bufferSize);
}

// =========================================

float Voltage(uint16_t value) {
    return (float)value * MILLIVOLTS_PER_STEP / 1000.0;
}