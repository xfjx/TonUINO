#include "settings.hpp"

#include <EEPROM.h>

#include "constants.hpp"
#include "logger.hpp"

namespace {

const int startAddressAdminSettings = sizeof(folderSettings::folder) * 100;

}

void Settings::writeByteToFlash(uint16_t address, uint8_t value) {
  EEPROM.update(address, value);
}

uint8_t Settings::readByteFromFlash(uint16_t address) {
  return EEPROM.read(address);
}

void Settings::clearEEPROM() {
  LOG(settings_log, s_info, F("Reset -> EEPROM wird gelöscht"));
  for (uint16_t i = 0; i < EEPROM.length(); i++) {
    writeByteToFlash(i, 0);
  }
}

void Settings::writeSettingsToFlash() {
  LOG(settings_log, s_debug, F("=== writeSettingsToFlash()"));
  EEPROM.put(startAddressAdminSettings, *this);
}

void Settings::resetSettings() {
  LOG(settings_log, s_debug, F("=== resetSettings()"));
  cookie              = cardCookie;
  version             =  2;
  maxVolume           = 25;
  minVolume           =  5;
  initVolume          = 15;
  eq                  =  1;
  locked              = false;
  standbyTimer        =  0;
  invertVolumeButtons = true;
  shortCuts[0].folder =  0;
  shortCuts[1].folder =  0;
  shortCuts[2].folder =  0;
  shortCuts[3].folder =  0;
  adminMenuLocked     =  0;
  adminMenuPin[0]     =  1;
  adminMenuPin[1]     =  1;
  adminMenuPin[2]     =  1;
  adminMenuPin[3]     =  1;
  pauseWhenCardRemoved= false;

  writeSettingsToFlash();
}

void Settings::migrateSettings(int oldVersion) {
  if (oldVersion == 1) {
    LOG(settings_log, s_info, F("=== migradeSettings() 1 -> 2"));
    version = 2;
    adminMenuLocked = 0;
    adminMenuPin[0] = 1;
    adminMenuPin[1] = 1;
    adminMenuPin[2] = 1;
    adminMenuPin[3] = 1;
    pauseWhenCardRemoved = false;
    writeSettingsToFlash();
  }
}

void Settings::loadSettingsFromFlash() {
  LOG(settings_log, s_debug, F("=== loadSettingsFromFlash()"));
  EEPROM.get(startAddressAdminSettings, *this);
  if (cookie != cardCookie)
    resetSettings();
  migrateSettings(version);

  LOG(settings_log, s_info, F("Version: "                ), version);
  LOG(settings_log, s_info, F("Maximal Volume: "         ), maxVolume);
  LOG(settings_log, s_info, F("Minimal Volume: "         ), minVolume);
  LOG(settings_log, s_info, F("Initial Volume: "         ), initVolume);
  LOG(settings_log, s_info, F("EQ: "                     ), eq);
  LOG(settings_log, s_info, F("Locked: "                 ), locked);
  LOG(settings_log, s_info, F("Sleep Timer: "            ), standbyTimer);
  LOG(settings_log, s_info, F("Inverted Volume Buttons: "), invertVolumeButtons);
  LOG(settings_log, s_info, F("Admin Menu locked: "      ), adminMenuLocked);
  LOG(settings_log, s_info, F("Admin Menu Pin: "         ), adminMenuPin[0], adminMenuPin[1], adminMenuPin[2], adminMenuPin[3]);
  LOG(settings_log, s_info, F("Pause when card removed: "), pauseWhenCardRemoved);
}

void Settings::writeFolderSettingToFlash(uint8_t folder, uint16_t track) {
  writeByteToFlash(folder, min(track, 0xff));
}

uint16_t Settings::readFolderSettingFromFlash(uint8_t folder) {
  return readByteFromFlash(folder);
}


