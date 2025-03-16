#include <TinyWireM.h>
#include <Tiny4kOLED.h>

// ================================== Defenitions ==================================
// SSD1306 Settings ==================================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

// ================================== Grapher Class ==================================
class Grapher
{
  private:
      uint8_t data[SCREEN_WIDTH];

  public:
      Grapher() {
          memset(data, 0, sizeof(data));
      }

      void apply(void (*genFunc)(uint8_t *)) {
          genFunc(data);
      }

      void Display(void (*prefetchFunc)()) {

          prefetchFunc();

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
};

// ================================== Predeclares ==================================
Grapher graph;

void genSaw(uint8_t *arr); // generate "//" lines
void genSINWave(uint8_t *arr); // generate sin() line
void genPick(uint8_t *arr); // generate "/\" line

void clean() {
  oled.clear();
}

void setup() {
  oled.begin(128, 64, sizeof(tiny4koled_init_128x64br), tiny4koled_init_128x64br);
  oled.clear();
  oled.on();
}

void loop() {
  graph.apply(genPick);
  graph.Display(clean);
  delay(3*1000);

  graph.apply(genSaw);
  graph.Display(clean);
  delay(3*1000);

  graph.apply(genSINWave);
  graph.Display(clean);
  delay(3*1000);
}


void genPick(uint8_t *arr) {
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        if (SCREEN_HEIGHT < x) {
          arr[x] = uint8_t(SCREEN_HEIGHT) - (x - uint8_t(SCREEN_HEIGHT));
          continue;
        }
        arr[x] = x;
    }
}

void genSaw(uint8_t *arr) {
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        if (SCREEN_HEIGHT < x) {
          arr[x] = x - uint8_t(SCREEN_HEIGHT);
          continue;
        }
        arr[x] = x;
    }
}

void genSINWave(uint8_t *arr) {
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        arr[x] = (uint8_t)( (sin(x * 2 * M_PI / SCREEN_WIDTH) * 0.5 + 0.5) * (SCREEN_HEIGHT - 1) );
    }
}