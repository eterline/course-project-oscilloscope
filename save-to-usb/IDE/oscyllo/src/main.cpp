// main.cpp
#include <Arduino.h>
#include <EncButton.h>
// -------------------------
#include <Transfer.h>
#include <Oscillo.h>
#include <Display.h>

// =========================

Button buttonLeft(D0);
Button buttonRight(D3);
Oscillo oscillo;

// ==================================================================

void setup() {
  DisableWIFI();
  StartSerial();

  RunDisplay();
  Logo();

  delay(500);
}

void loop() {

  // call buttons ticks updates
  buttonLeft.tick();
  buttonRight.tick();

  oscillo.DoRead();

  // Mode selector 
  if (buttonLeft.holding() && buttonRight.click()) {
    NextMode();
  }

  switch (ModeNow())
  {
    case READING:
      if (Once()) {
        ShowBigText("READING MODE");
        delay(500);
        oscDrawData();
      }
      if (!buttonLeft.holding() && buttonRight.click()) { UpOscilloScale(); }

      oscDrawGraph(
        oscillo.Data,
        oscillo.Duration(),
        oscillo.SampleRate(),
        oscillo.UnwrapFreq(),
        oscillo.Max(),
        oscillo.Average()
      );

      break;

    // TODO: in process to make saving
    case SAVES:
      if (Once()) {
        ShowBigText("SAVES IN");
        delay(500);
      }

      break;

    case USB:
      char json[JSON_LEN];
      if (Once()) {
        ShowBigText("USB MODE");
        delay(500);
        usbDrawData(SerialSpeed(), SerialSentBytes());
      }
      if (!buttonLeft.holding() && buttonRight.click()) { NextBaud(); }

      oscillo.Serialize(json, JSON_LEN); SerialSend(json);
      usbDrawInfo(SerialSpeed());
      break;

    default:
      NextMode();
  }
}

// ==================================================================