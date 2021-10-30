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

  button get_button();
  void wait_for_no_button();
  bool is_reset();
  bool is_break();
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
  #ifdef FIVEBUTTONS
  bool ignoreButtonFour  = false;
  bool ignoreButtonFive  = false;
  #endif

  const Settings& settings;
};

#endif /* SRC_BUTTONS_HPP_ */
