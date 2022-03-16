#ifndef SRC_BUTTONS_HPP_
#define SRC_BUTTONS_HPP_

#include <Arduino.h>
#include <JC_Button.h>

#include "settings.hpp"
#include "constants.hpp"

enum class buttonRaw: uint8_t {
  none,
  pause,
  pauseLong,
  up,
  upLong,
  down,
  downLong,
  allLong,
#ifdef FIVEBUTTONS
  four,
  five,
#endif
  start,
};

enum class buttonCmd: uint8_t {
  none,
  admin,
  pause,
  track,
  volume_up,
  volume_down,
  next,
  previous,
  start,
};

class Buttons {
public:
  Buttons(const Settings& settings);

  buttonRaw getButtonRaw();
  buttonCmd getButtonCmd(buttonRaw b);
  static uint8_t   getButtonCode(buttonRaw b);
  bool isReset();
  bool isNoButton();

private:

  void readButtons();

  Button buttonPause;
  Button buttonUp   ;
  Button buttonDown ;
  #ifdef FIVEBUTTONS
  Button  buttonFour;
  Button  buttonFive;
  #endif
  bool ignorePauseButton = false;
  bool ignoreUpButton    = false;
  bool ignoreDownButton  = false;

  const Settings& settings;
};

#endif /* SRC_BUTTONS_HPP_ */
