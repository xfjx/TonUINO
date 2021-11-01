#ifndef SRC_BUTTONS_HPP_
#define SRC_BUTTONS_HPP_

#include <Arduino.h>
#include <JC_Button.h>

#include "settings.hpp"

// uncomment the below line to enable five button support
//#define FIVEBUTTONS

enum class button {
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

  button getButton();
  void waitForNoButton();
  bool isReset();
  bool isBreak();
  bool askCode(Settings::pin_t &code);

private:

  void readButtons();

  Button pauseButton;
  Button    upButton;
  Button  downButton;
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
