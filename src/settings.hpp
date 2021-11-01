#ifndef SRC_SETTINGS_HPP_
#define SRC_SETTINGS_HPP_

#include <Arduino.h>

#include "array.hpp"
#include "chip_card.hpp"

// admin settings stored in eeprom
struct Settings {
  typedef array<folderSettings, 4> shortCuts_t;
  typedef array<uint8_t       , 4> pin_t;

  void    writeByteToFlash (uint16_t address, uint8_t value);
  uint8_t readByteFromFlash(uint16_t address);

  void clearEEPROM();

  void writeSettingsToFlash();
  void resetSettings();
  void migrateSettings(int oldVersion);
  void loadSettingsFromFlash();

  void     writeFolderSettingToFlash (uint8_t folder, uint16_t track);
  uint16_t readFolderSettingFromFlash(uint8_t folder);

  uint32_t    cookie;
  byte        version;
  uint8_t     maxVolume;
  uint8_t     minVolume;
  uint8_t     initVolume;
  uint8_t     eq;
  bool        locked;
  long        standbyTimer;
  bool        invertVolumeButtons;
  shortCuts_t shortCuts;
  uint8_t     adminMenuLocked;
  pin_t       adminMenuPin;
};

#endif /* SRC_SETTINGS_HPP_ */
