#include <TinyWireM.h>

// ================================== Defenitions ==================================

// Hex commands ==================================
#define WIRE_DISABLE_DISPLAY 0xAE
#define WIRE_ENABLE_DISPLAY 0xAF

#define WIRE_COMMAND_MODE 0x00

// SSD1306 Settings ==================================
#define OLED_ADDR      0x3C  // I²C address
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define PAGE_COUNT    (SCREEN_HEIGHT / 8)

// ================================== Grapher Class ==================================
class Grapher
{
    private:

        uint8_t valueArr[SCREEN_WIDTH]; // MAIN DATA ARRAY

        void writeBlob(uint8_t comma, uint8_t value) {
            TinyWireM.beginTransmission(OLED_ADDR);
            TinyWireM.write(comma);
            TinyWireM.write(value);
            TinyWireM.endTransmission();
        }

        void sendCommand(uint8_t cmd) {
            writeBlob(0x00, cmd);
        }

        void sendData(uint8_t d) {
            writeBlob(0x40, d);
        }

        void setDot(uint8_t page, uint8_t small_bit, uint8_t hight_bit) {
            sendCommand(page);              // Page set
            sendCommand(small_bit);         // Set X (small bit)
            sendCommand(hight_bit);         // Set X (hight bit)
        }

    public:
        Grapher() {
            memset(valueArr, 0, sizeof(valueArr));
        }

        void Init() {

            TinyWireM.begin(); // Start TinyWire

            uint8_t hexArr[SCREEN_WIDTH] = {
                WIRE_DISABLE_DISPLAY,
                0xD5, 0x80,    // Div line
                0xA8, 0x3F,    // Set MUX (64 строки)
                0xD3, 0x00,    // Display offset
                0x40,          // Start string
                0x8D, 0x14,    // Charge enable
                0x20, 0x00,    // Horizontal mode addrs
                0xA1,          // Display horizontal
                0xC8,          // Display vert
                0xDA, 0x12,    // Setting COM
                0x81, 0xCF,    // Contrast
                0xD9, 0xF1,    // Retract phase
                0xDB, 0x40,    // Level VCOM
                0xA4,          // Diplay from RAM
                0xA6,          // Usual mode
                WIRE_ENABLE_DISPLAY,
            };

            for (uint8_t x = 0; x < sizeof(hexArr); x++) {
                sendCommand(hexArr[x]);
            };
        }

        void apply(void (*genFunc)(uint8_t *)) {
            genFunc(valueArr);
        }
        
        void draw() {

            clear();

            for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
                uint8_t y = valueArr[x];

                if (y >= SCREEN_HEIGHT) continue;

                uint8_t page = y / 8;
                uint8_t bit = y % 8;

                setDot(0xB0 + page, (x & 0x0F), 0x10 | (x >> 4));

                sendData(1 << bit);
            }
        }

        void clear() {
            for (uint8_t page = 0; page < PAGE_COUNT; page++) {

                setDot(0xB0 + page, 0x00, 0x10);

                for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
                    sendData(0x00);  // Fill zeros
                }
            }
        }
};

// ================================== Predeclares ==================================
Grapher graph;

void genSaw(uint8_t *arr); // generate "//" lines
void genSINWave(uint8_t *arr); // generate sin() line
void genPick(uint8_t *arr); // generate "/\" line

void setup() {
    graph.Init();
}

void loop() {
    graph.apply(genSaw);
    graph.draw();
    delay(2000);
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