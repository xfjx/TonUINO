#include "buttons.hpp"

#include "constants.hpp"
#include "logger.hpp"

namespace {
constexpr bool buttonPinIsActiveLow = (buttonPinType == levelType::activeLow);
}

Buttons::Buttons()
: CommandSource()
//            pin             dbTime        puEnable              invert
, buttonPause(buttonPausePin, buttonDbTime, buttonPinIsActiveLow, buttonPinIsActiveLow)
, buttonUp   (buttonUpPin   , buttonDbTime, buttonPinIsActiveLow, buttonPinIsActiveLow)
, buttonDown (buttonDownPin , buttonDbTime, buttonPinIsActiveLow, buttonPinIsActiveLow)
#ifdef FIVEBUTTONS
, buttonFour (buttonFourPin , buttonDbTime, buttonPinIsActiveLow, buttonPinIsActiveLow)
, buttonFive (buttonFivePin , buttonDbTime, buttonPinIsActiveLow, buttonPinIsActiveLow)
#endif
{
  buttonPause.begin();
  buttonUp   .begin();
  buttonDown .begin();
#ifdef FIVEBUTTONS
  buttonFour .begin();
  buttonFive .begin();
#endif
}

commandRaw Buttons::getCommandRaw() {
  commandRaw ret = commandRaw::none;
  readButtons();
  if ((  buttonPause.pressedFor(buttonLongPress)
      || buttonUp   .pressedFor(buttonLongPress)
      || buttonDown .pressedFor(buttonLongPress)
      )
     && buttonPause.isPressed()
     && buttonUp   .isPressed()
     && buttonDown .isPressed()) {
    ret = commandRaw::allLong;
  }

  else if (buttonPause.wasReleased()) {
    if (not ignorePauseButton)
      ret = commandRaw::pause;
    else
      ignorePauseButton = false;
  }

  else if (buttonPause.pressedFor(buttonLongPress) && not ignorePauseButton) {
    ret = commandRaw::pauseLong;
    ignorePauseButton = true;
  }

  else if (buttonUp.wasReleased()) {
    if (!ignoreUpButton) {
      ret = commandRaw::up;
    }
    else
      ignoreUpButton = false;
  }

  else if (buttonUp.pressedFor(buttonLongPress) && not ignoreUpButton) {
    ret = commandRaw::upLong;
    ignoreUpButton = true;
  }

  else if (buttonDown.wasReleased()) {
    if (!ignoreDownButton) {
      ret = commandRaw::down;
    }
    else
      ignoreDownButton = false;
  }

  else if (buttonDown.pressedFor(buttonLongPress) && not ignoreDownButton) {
    ret = commandRaw::downLong;
    ignoreDownButton = true;
  }

#ifdef FIVEBUTTONS
  else if (buttonFour.wasReleased()) {
    ret = commandRaw::four;
  }

  else if (buttonFive.wasReleased()) {
    ret = commandRaw::five;
  }
#endif

  if (ret != commandRaw::none) {
    LOG(button_log, s_debug, F("Button raw: "), static_cast<uint8_t>(ret));
  }
  return ret;
}

bool Buttons::isReset() {
  constexpr int buttonActiveLevel = getLevel(buttonPinType, level::active);
  return (digitalRead(buttonPausePin) == buttonActiveLevel &&
          digitalRead(buttonUpPin   ) == buttonActiveLevel &&
          digitalRead(buttonDownPin ) == buttonActiveLevel );
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



