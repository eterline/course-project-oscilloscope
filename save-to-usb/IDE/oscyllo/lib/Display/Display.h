// Display.h
#include <SPI.h>
#include <ESP8266WiFi.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include <Oscillo.h>

// ==================================================

#ifndef Display_h
#define Display_h

// ==================================================

#define TFT_CS        D8
#define TFT_RST       D6
#define TFT_DC        D1

// ==================================================

enum Mode {
    READING,
    SAVES,
    USB,
    MODE_COUNT
};

extern Mode __modeIs;
extern bool __doOnce;

Mode ModeNow();
void NextMode();
bool Once();

void DisableWIFI();
void clearDisplay(uint16_t color = ST77XX_BLACK);

void UpOscilloScale();
void ResetOscilloScale();

void RunDisplay();
void Logo();
void ShowBigText(const char * text);

void oscDrawData();
void oscDrawGraph(uint16_t* arr, unsigned long duration, unsigned long samples, unsigned long freq, uint16_t max_v, uint16_t avg_v);

void usbDrawData(unsigned long baud, unsigned long sent);
void usbDrawInfo(unsigned long baud);

// ==================================================
#endif