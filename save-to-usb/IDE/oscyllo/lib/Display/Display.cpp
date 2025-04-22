#include <Display.h>

// ==================================================

#define DEFAULT_SCALE 1
#define FRAME_TIME    100 // rate - 10 fps

#define DISPLAY_WIDTH     160
#define DISPLAY_HEIGHT    128

// ==================================================

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

Mode __modeIs             = READING;
bool __doOnce             = true;
unsigned long __mode_time = 0;
uint8_t __graph_scale     = 1;

unsigned long __frame_time_now;
unsigned long __old_frame_time = 0;

// ==================================================

bool allowDrawFrame() {
  __frame_time_now = millis();
  bool allow = __frame_time_now - __old_frame_time >= FRAME_TIME;
  if (allow)
  {
    __old_frame_time = __frame_time_now;
  }
  return allow;
}

bool allowDrawFrameIn(unsigned long t) {
  __frame_time_now = millis();
  bool allow = __frame_time_now - __old_frame_time >= t;
  if (allow)
  {
    __old_frame_time = __frame_time_now;
  }
  return allow;
}

// ==================================================

void UpOscilloScale() {
  __graph_scale = ((__graph_scale + 1) % 8);
  if (__graph_scale == 0) __graph_scale = 1;
  clearDisplay();
  oscDrawData();
}

void ResetOscilloScale() {
  __graph_scale = 1;
}

// ==================================================

Mode ModeNow() {
  return __modeIs; 
}

void NextMode() { 
  __modeIs = static_cast<Mode>((__modeIs + 1) % MODE_COUNT); 
  __doOnce = true;
}

bool Once() {
  if (__doOnce) 
  {
    __doOnce = false;
    return true;
  }
  return false;
}

// ==================================================

void clearDisplay(uint16_t color) {
  tft.fillScreen(color);
  tft.setCursor(0, 0);
}

void DisableWIFI() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(10);  
}

// ==================================================

void RunDisplay() {
  __old_frame_time = millis();

  tft.init(240, 240);
  tft.setSPISpeed(40000000);
  tft.setAddrWindow(0, 0, DISPLAY_HEIGHT, DISPLAY_WIDTH);

  tft.invertDisplay(false);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
}

void Logo()
{
  for (uint8_t i = 0; i < 64; i++)
  {
    uint8_t end_x = map(i, 0, 63, 0, DISPLAY_WIDTH);
    uint8_t start_x = map(i, 0, 63, 64, 80);
    uint16_t color = (ST77XX_CYAN + 2 * i);

    tft.drawLine(start_x, 10, end_x, 80, color);
  }

  tft.setCursor(10, 90);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  
  tft.print("OSCYLLO");
}

void ShowBigText(const char * text)
{
  clearDisplay();

  uint8_t end_x, start_x, color;
  for (uint8_t i = 0; i < 64; i++)
  {
    end_x   = map(i, 0, 63, 0, DISPLAY_WIDTH);
    start_x = map(i, 0, 63, 64, 80);
    color   = (ST77XX_CYAN + i);

    tft.drawLine(start_x, 10, end_x, 80, color);
  }

  tft.setCursor(10, 90);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  
  tft.print(text);
  tft.setTextSize(1);
}

// =============================================
// Reading mode functions

// drawDottedLine - draws line with certain len
void drawDottedLine(int16_t y_x, int16_t l, bool vertical = false) {
  for (int i = 0; i < l; i += 4) {
    if (vertical) {
      tft.drawFastVLine(y_x, i, 2, 0xf4e0); // Vertical segment
    } else {
      tft.drawFastHLine(i, y_x, 2, 0xf4e0); // Horizontal segment
    }
  }
}

// oscDrawData - display oscilloscope base menu text
void oscDrawData()
{
  clearDisplay();
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);

  tft.setCursor(2, 2);
  tft.print("> FQ | duration | KSmpl/S");

  tft.drawFastHLine(0, 22, DISPLAY_WIDTH, 0xc1d3);
  tft.drawFastHLine(0, 112, DISPLAY_WIDTH, 0xc1d3);

  tft.setCursor(2, 116);
  tft.print("X");
  tft.print(__graph_scale);
}

void oscDrawGraph(uint16_t* arr, unsigned long duration, unsigned long samples, unsigned long freq, uint16_t max_v, uint16_t avg_v) {
  if (allowDrawFrame()) // frame rate stop - 2 fps
  {
    static uint16_t old_map[GRAPH_LEN];

    drawDottedLine(44, DISPLAY_WIDTH);
    drawDottedLine(66, DISPLAY_WIDTH);
    drawDottedLine(88, DISPLAY_WIDTH);

    float dur = (float)duration / 1000.0;

    tft.setTextColor(ST77XX_GREEN);
    
    tft.setCursor(2, 12);
    tft.fillRect(2, 12, DISPLAY_WIDTH, 10, ST77XX_BLACK);
    tft.printf(" %luHz | %.2fms | %lu", freq * __graph_scale, dur / __graph_scale, samples / 1000);
    
    tft.setCursor(20, 116);
    tft.fillRect(20, 116, DISPLAY_WIDTH, 10, ST77XX_BLACK);
    tft.printf("max: %.2fV | avg %.2fV", Voltage(max_v), Voltage(avg_v));

    tft.startWrite();
    for (size_t i = 0; i < (GRAPH_LEN - 1) / __graph_scale; i++) {

      if (arr[i] != old_map[i] || arr[i + 1] != old_map[i + 1]) {
        uint8_t x1 = map(i, 0, GRAPH_LEN / __graph_scale - 2, 0, DISPLAY_WIDTH);
        uint8_t x2 = map(i + 1, 0, GRAPH_LEN / __graph_scale - 2, 0, DISPLAY_WIDTH);


        uint8_t y1_old = map(old_map[i], 0, 1023, 111, 23);
        uint8_t y2_old = map(old_map[i + 1], 0, 1023, 111, 23);
        tft.writeLine(x1, y1_old, x2, y2_old, ST77XX_BLACK);

        uint8_t y1_new = map(arr[i], 0, 1023, 111, 23);
        uint8_t y2_new = map(arr[i + 1], 0, 1023, 111, 23);
        tft.writeLine(x1, y1_new, x2, y2_new, 0x6b39);
      }
    }
    tft.endWrite();

    memcpy(old_map, arr, GRAPH_LEN * sizeof(uint16_t));
  }
}

// =============================================
// USB mode functions

void printSentSize(unsigned long sent) {
  if (sent >= 1024 * 1024) {
    float mb = sent / 1024.0 / 1024.0;
    tft.printf("%.2f MB", mb);
  } else if (sent >= 1024) {
    float kb = sent / 1024.0;
    tft.printf("%.1f KB", kb);
  } else {
    tft.print(sent);
    tft.print(" B");
  }
}

void usbDrawData(unsigned long baud, unsigned long sent)
{
  clearDisplay();

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(2, 2);
  tft.println("> USB mode info");

  tft.drawFastHLine(0, 15, DISPLAY_WIDTH, ST77XX_YELLOW);

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_ORANGE);

  tft.setCursor(2, 25); tft.println("[Baud]");
  // tft.setCursor(2, 85); tft.println("[Sent]");

  tft.setTextColor(ST77XX_GREEN);

  tft.fillRect(0, 45, DISPLAY_WIDTH, 20, ST77XX_BLACK);
  tft.setCursor(5, 45);
  tft.print(baud);

  // tft.fillRect(0, 105, DISPLAY_WIDTH, 20, ST77XX_BLACK);
  // tft.setCursor(5, 105);
  // printSentSize(sent);
}

void usbDrawInfo(unsigned long baud)
{
  static unsigned long __old_baud = 0;
  // static unsigned long __old_sent = 0;

  if (allowDrawFrameIn(1000)) // frame rate stop - 1 fps
  {
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_GREEN);

    if (__old_baud != baud) {
      __old_baud = baud;
      tft.fillRect(0, 45, DISPLAY_WIDTH, 20, ST77XX_BLACK);
      tft.setCursor(5, 45);
      tft.print(baud);
    }

    // if (__old_sent != sent) {
    //   __old_sent = sent;
    //   tft.fillRect(0, 105, DISPLAY_WIDTH, 20, ST77XX_BLACK);
    //   tft.setCursor(5, 105);
    //   printSentSize(sent);
    // }
  }
}