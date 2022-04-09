#include "commands.hpp"

#include "constants.hpp"
#include "logger.hpp"

Commands::Commands(const Settings& settings, CommandSource* source1, CommandSource* source2, CommandSource* source3)
: settings(settings)
, sources()
{
  sources[0] = source1;
  sources[1] = source2;
  sources[2] = source3;
}

commandRaw Commands::getCommandRaw() {
  for (auto source: sources) {
    if (source != nullptr) {
      const commandRaw c = source->getCommandRaw();
      if (c != commandRaw::none)
        return c;
    }
  }
  return commandRaw::none;
}

command Commands::getCommand(commandRaw b) {
  command ret = command::none;

  switch (b) {
  case commandRaw::none     : ret = command::none                                                                  ; break;
  case commandRaw::pause    : ret = command::pause                                                                 ; break;
  case commandRaw::pauseLong: ret = command::track                                                                 ; break;
  case commandRaw::up       : ret = (!settings.invertVolumeButtons) ? command::next        : command::volume_up  ; break;
  case commandRaw::upLong   : ret = (!settings.invertVolumeButtons) ? command::volume_up   : command::next       ; break;
  case commandRaw::down     : ret = (!settings.invertVolumeButtons) ? command::previous    : command::volume_down; break;
  case commandRaw::downLong : ret = (!settings.invertVolumeButtons) ? command::volume_down : command::previous   ; break;
  case commandRaw::allLong  : ret = command::admin                                                                 ; break;
#ifdef FIVEBUTTONS
  case commandRaw::four     : ret = (!settings.invertVolumeButtons) ? command::volume_up   : command::next       ; break;
  case commandRaw::five     : ret = (!settings.invertVolumeButtons) ? command::volume_down : command::previous   ; break;
#endif
  case commandRaw::start    : ret = command::start;                                                                ; break;
  }

  if (ret != command::none) {
    LOG(button_log, s_debug, F("Command: "), static_cast<uint8_t>(ret));
  }
  return ret;
}

uint8_t Commands::getButtonCode(commandRaw b) {
  switch (b) {
  case commandRaw::pause: return 1;
  case commandRaw::up   : return 2;
  case commandRaw::down : return 3;
  default               : return 0;
  }
}

