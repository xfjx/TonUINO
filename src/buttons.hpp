#ifndef SRC_BUTTONS_HPP_
#define SRC_BUTTONS_HPP_

#include <Arduino.h>
#include <JC_Button.h>

#include "commands.hpp"
#include "constants.hpp"

class Buttons: public CommandSource {
public:
  Buttons();

  commandRaw getCommandRaw() override;
  bool isReset();

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

};

#endif /* SRC_BUTTONS_HPP_ */
