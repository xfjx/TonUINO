#ifndef SRC_BUTTONS_HPP_
#define SRC_BUTTONS_HPP_

#include <Arduino.h>
#include <JC_Button.h>

#include "settings.hpp"
#include "constants.hpp"

enum class buttonRaw {
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
};

enum class buttonCmd {
  none,
  admin,
  pause,
  track,
  volume_up,
  volume_down,
  next,
  previous,
};

class Buttons {
public:
  Buttons(const Settings& settings);

  buttonRaw getButtonRaw();
  buttonCmd getButtonCmd();
  void waitForNoButton();
  bool isReset();
  bool isBreak();
  bool askCode(Settings::pin_t &code);

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
