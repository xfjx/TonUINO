#include "buttons.hpp"

namespace {

const uint32_t LONG_PRESS = 1000;
const uint8_t buttonPause   = A0;
const uint8_t buttonUp      = A1;
const uint8_t buttonDown    = A2;

#ifdef FIVEBUTTONS
const uint8_t buttonFourPin = A3;
const uint8_t buttonFivePin = A4;
#endif

}

Buttons::Buttons(const Settings& settings)
: pauseButton(buttonPause  )
,    upButton(buttonUp     )
,  downButton(buttonDown   )
#ifdef FIVEBUTTONS
, buttonFour (buttonFourPin);
, buttonFive (buttonFivePin);
#endif
, settings(settings)
{
  pinMode(buttonPause  , INPUT_PULLUP);
  pinMode(buttonUp     , INPUT_PULLUP);
  pinMode(buttonDown   , INPUT_PULLUP);
#ifdef FIVEBUTTONS
  pinMode(buttonFourPin, INPUT_PULLUP);
  pinMode(buttonFivePin, INPUT_PULLUP);
#endif
}

button Buttons::getButton() {
  button ret = button::none;
  readButtons();
  if ((  pauseButton.pressedFor(LONG_PRESS)
      || upButton   .pressedFor(LONG_PRESS)
      || downButton .pressedFor(LONG_PRESS)
      )
     && pauseButton.isPressed()
     && upButton   .isPressed()
     && downButton .isPressed())
    ret = button::admin;

  else if (pauseButton.wasReleased()) {
    if (not ignorePauseButton)
      ret = button::pause;
    else
      ignorePauseButton = false;
  }

  else if (pauseButton.pressedFor(LONG_PRESS) && not ignorePauseButton) {
    ret = button::track;
    ignorePauseButton = true;
  }

  else if (upButton.wasReleased()) {
    if (!ignoreUpButton) {
      if (!settings.invertVolumeButtons)
        ret = button::next;
      else
        ret = button::volume_up;
    }
    else
      ignoreUpButton = false;
  }

  else if (upButton.pressedFor(LONG_PRESS) && not ignoreUpButton) {
#ifndef FIVEBUTTONS
    if (!settings.invertVolumeButtons)
      ret = button::volume_up;
    else
      ret = button::next;
    ignoreUpButton = true;
#endif
  }

  else if (downButton.wasReleased()) {
    if (!ignoreDownButton) {
      if (!settings.invertVolumeButtons)
        ret = button::previous;
      else
        ret = button::volume_down;
    }
    else
      ignoreDownButton = false;
  }

  else if (downButton.pressedFor(LONG_PRESS) && not ignoreDownButton) {
#ifndef FIVEBUTTONS
    if (!settings.invertVolumeButtons)
      ret = button::volume_down;
    else
      ret = button::previous;
    ignoreDownButton = true;
#endif
  }

#ifdef FIVEBUTTONS
  else if (buttonFour.wasReleased()) {
    if (!settings.invertVolumeButtons)
      ret = button::volume_up;
    else
      ret = button::next;
  }

  else if (buttonFive.wasReleased()) {
    if (!settings.invertVolumeButtons)
      ret = button::volume_down;
    else
      ret = button::previous;
  }
#endif

//  if (ret != button::none) {
//    Serial.print(F("Button: ")); Serial.println(static_cast<uint8_t>(ret));
//  }
  return ret;
}

void Buttons::waitForNoButton() {
  do {
    readButtons();
  } while (  pauseButton.isPressed()
          || upButton   .isPressed()
          || downButton .isPressed()
#ifdef FIVEBUTTONS
          || buttonFour .isPressed()
          || buttonFive .isPressed()
#endif
          );
  ignorePauseButton = false;
  ignoreUpButton    = false;
  ignoreDownButton  = false;
}

bool Buttons::isReset() {
  return (digitalRead(buttonPause) == LOW &&
          digitalRead(buttonUp)    == LOW &&
          digitalRead(buttonDown)  == LOW );
}

bool Buttons::isBreak() {
  readButtons();
  if (upButton.wasReleased() || downButton.wasReleased()) {
    Serial.print(F("Abgebrochen!"));
    return true;
  }
  return false;
}

bool Buttons::askCode(Settings::pin_t &code) {
  uint8_t x = 0;
  while (x < 4) {
    readButtons();
    if (pauseButton.pressedFor(LONG_PRESS))
      return false;
    if (pauseButton.wasReleased())
      code[x++] = 1;
    if (upButton.wasReleased())
      code[x++] = 2;
    if (downButton.wasReleased())
      code[x++] = 3;
  }
  return true;
}

void Buttons::readButtons() {
  pauseButton.read();
  upButton   .read();
  downButton .read();
#ifdef FIVEBUTTONS
  buttonFour .read();
  buttonFive .read();
#endif
}



