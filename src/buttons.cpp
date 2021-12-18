#include "buttons.hpp"

#include "constants.hpp"
#include "logger.hpp"

namespace {
const bool buttonPinIsActiveLow = (buttonPinType == levelType::activeLow);
}

Buttons::Buttons(const Settings& settings)
//            pin             dbTime        puEnable              invert
: buttonPause(buttonPausePin, buttonDbTime, buttonPinIsActiveLow, buttonPinIsActiveLow)
, buttonUp   (buttonUpPin   , buttonDbTime, buttonPinIsActiveLow, buttonPinIsActiveLow)
, buttonDown (buttonDownPin , buttonDbTime, buttonPinIsActiveLow, buttonPinIsActiveLow)
#ifdef FIVEBUTTONS
, buttonFour (buttonFourPin , buttonDbTime, buttonPinIsActiveLow, buttonPinIsActiveLow)
, buttonFive (buttonFivePin , buttonDbTime, buttonPinIsActiveLow, buttonPinIsActiveLow)
#endif
, settings(settings)
{
  buttonPause.begin();
  buttonUp   .begin();
  buttonDown .begin();
#ifdef FIVEBUTTONS
  buttonFour .begin();
  buttonFive .begin();
#endif
}

button Buttons::getButton() {
  button ret = button::none;
  readButtons();
  if ((  buttonPause.pressedFor(buttonLongPress)
      || buttonUp   .pressedFor(buttonLongPress)
      || buttonDown .pressedFor(buttonLongPress)
      )
     && buttonPause.isPressed()
     && buttonUp   .isPressed()
     && buttonDown .isPressed())
    ret = button::admin;

  else if (buttonPause.wasReleased()) {
    if (not ignorePauseButton)
      ret = button::pause;
    else
      ignorePauseButton = false;
  }

  else if (buttonPause.pressedFor(buttonLongPress) && not ignorePauseButton) {
    ret = button::track;
    ignorePauseButton = true;
  }

  else if (buttonUp.wasReleased()) {
    if (!ignoreUpButton) {
      if (!settings.invertVolumeButtons)
        ret = button::next;
      else
        ret = button::volume_up;
    }
    else
      ignoreUpButton = false;
  }

  else if (buttonUp.pressedFor(buttonLongPress) && not ignoreUpButton) {
#ifndef FIVEBUTTONS
    if (!settings.invertVolumeButtons)
      ret = button::volume_up;
    else
      ret = button::next;
    ignoreUpButton = true;
#endif
  }

  else if (buttonDown.wasReleased()) {
    if (!ignoreDownButton) {
      if (!settings.invertVolumeButtons)
        ret = button::previous;
      else
        ret = button::volume_down;
    }
    else
      ignoreDownButton = false;
  }

  else if (buttonDown.pressedFor(buttonLongPress) && not ignoreDownButton) {
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

  if (ret != button::none) {
    LOG(button_log, s_debug, F("Button: "), static_cast<uint8_t>(ret));
  }
  return ret;
}

void Buttons::waitForNoButton() {
  do {
    readButtons();
  } while (  buttonPause.isPressed()
          || buttonUp   .isPressed()
          || buttonDown .isPressed()
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
  const int buttonActiveLevel = getLevel(buttonPinType, level::active);
  return (digitalRead(buttonPausePin) == buttonActiveLevel &&
          digitalRead(buttonUpPin   ) == buttonActiveLevel &&
          digitalRead(buttonDownPin ) == buttonActiveLevel );
}

bool Buttons::isBreak() {
  readButtons();
  if (buttonUp.wasReleased() || buttonDown.wasReleased()) {
    LOG(button_log, s_info, F("Abgebrochen!"));
    return true;
  }
  return false;
}

bool Buttons::askCode(Settings::pin_t &code) {
  uint8_t x = 0;
  while (x < 4) {
    readButtons();
    if (buttonPause.pressedFor(buttonLongPress))
      return false;
    if (buttonPause.wasReleased())
      code[x++] = 1;
    if (buttonUp.wasReleased())
      code[x++] = 2;
    if (buttonDown.wasReleased())
      code[x++] = 3;
  }
  return true;
}

void Buttons::readButtons() {
  buttonPause.read();
  buttonUp   .read();
  buttonDown .read();
#ifdef FIVEBUTTONS
  buttonFour .read();
  buttonFive .read();
#endif
}



