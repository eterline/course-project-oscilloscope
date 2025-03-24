#include <TinyWireM.h>
#include <Tiny4kOLED.h>
#include <GyverButton.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define STORE_SIZE 2   // Reduced to save memory

#define MOD_CHANGE_PIN PB4
#define ADC_READ_PIN PB3

typedef void (*generatorFunc)(uint8_t*);

class Grapher {
  private:
      uint8_t data[SCREEN_WIDTH];  // Only one screen data buffer

  public:
      Grapher() { memset(data, 0, sizeof(data)); }

      void apply(generatorFunc genFunc) {genFunc(data); }

      void display() {
          oled.clear(); 
          for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {

              uint8_t y = uint8_t(SCREEN_HEIGHT) - data[x];

              // overflow check ====
              if (y >= SCREEN_HEIGHT) continue;

              // page calc =========
              uint8_t page = y / 8;
              uint8_t bit = y % 8;

              // ==================
              oled.setCursor(x, page);
              oled.startData();
              oled.sendData(1 << bit);
              oled.endData();
          }
      }

      uint8_t* getData() { return data; }
};

class MilTimer {
  private:
      unsigned long tNext = 0, intervalTime;

  public:
      MilTimer(unsigned long time) : intervalTime(time) {}
      bool isDone() {
          unsigned long now = millis();
          if (now - tNext >= intervalTime) {
              tNext = now;
              return true;
          }
          return false;
      }
};

class MemStore {
  private:
      uint8_t storeArr[STORE_SIZE][SCREEN_WIDTH];  // Reduced store size to 2 for less memory usage
      uint8_t currentIndex = 0;

  public:
      uint8_t* getArr() { return storeArr[currentIndex]; }

      void next() { if (++currentIndex >= STORE_SIZE) currentIndex = 0; }

      void push(uint8_t* src) { memcpy(storeArr[currentIndex], src, SCREEN_WIDTH); }
};

Grapher graph;
MilTimer timer1s(1000);
MemStore store;
GButton modeBtn(MOD_CHANGE_PIN);
// GButton ctrlBtn(CTRL_PIN);
bool streamMode = true;

void setup() {
    pinMode(PB1, OUTPUT);
    oled.begin(128, 64, sizeof(tiny4koled_init_128x64br), tiny4koled_init_128x64br);
    oled.clear();
    oled.on();
}

void loop() {
    modeBtn.tick();
    // ctrlBtn.tick();


    // if (ctrlBtn.isSingle()) {
    //     graph.apply(readTemp);
    //     graph.display();
    // }

    if (modeBtn.isHold()) store.next();
    if (modeBtn.isSingle()) {
      digitalWrite(PB1, HIGH);
      streamMode = !streamMode;
      // if (!streamMode) {
      //   graph.apply(readTemp);
      //   graph.display();
      // }
    }

    if (timer1s.isDone() && streamMode) {
        graph.apply(osciReadLine);
        graph.display();
        store.push(graph.getData());
    }
}

void osciReadLine(uint8_t *arr) {
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        arr[x] = analogRead(ADC_READ_PIN) - 1;
    }
}

void readTemp(uint8_t *arr) {
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        arr[x] = store.getArr()[x];
    }
}
