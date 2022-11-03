#include "serial_input.hpp"

#include "constants.hpp"
#include "logger.hpp"

SerialInput::SerialInput()
: CommandSource()
{
}

commandRaw SerialInput::getCommandRaw() {
  commandRaw ret = commandRaw::none;
  if (Serial.available() > 0) {
    int optionSerial = Serial.parseInt();
    switch (optionSerial) {
    case 2: ret = commandRaw::down     ; break;
    case 8: ret = commandRaw::up       ; break;
    case 3: ret = commandRaw::downLong ; break;
    case 9: ret = commandRaw::upLong   ; break;
    case 5: ret = commandRaw::pause    ; break;
    case 6: ret = commandRaw::pauseLong; break;
    case 4: ret = commandRaw::allLong  ; break;
    default:                             break;
    }
  }
  return ret;
}

