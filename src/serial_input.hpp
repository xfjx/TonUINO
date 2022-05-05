#ifndef SRC_SERIAL_INPUT_HPP_
#define SRC_SERIAL_INPUT_HPP_

#include <Arduino.h>

#include "commands.hpp"
#include "constants.hpp"

class SerialInput: public CommandSource {
public:
  SerialInput();

  commandRaw getCommandRaw() override;
};

#endif /* SRC_SERIAL_INPUT_HPP_ */
