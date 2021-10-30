#include "settings.hpp"

#include <EEPROM.h>

namespace {

const int startAddressAdminSettings = sizeof(folderSettings::folder) * 100;

}

void Settings::clearEEPROM() {
  Serial.println(F("Reset -> EEPROM wird gelöscht"));
  for (unsigned int i = 0; i < EEPROM.length(); i++) {
    writeFolderSettingToFlash(i, 0);
  }
}

void Settings::writeSettingsToFlash() {
  Serial.println(F("=== writeSettingsToFlash()"));
  EEPROM.put(startAddressAdminSettings, *this);
}

void Settings::resetSettings() {
  Serial.println(F("=== resetSettings()"));
  cookie              = cardCookie;
  version             =  2;
  maxVolume           = 25;
  minVolume           =  5;
  initVolume          = 15;
  eq                  =  1;
  locked              = false;
  standbyTimer        = 0;
  invertVolumeButtons = true;
  shortCuts[0].folder = 0;
  shortCuts[1].folder = 0;
  shortCuts[2].folder = 0;
  shortCuts[3].folder = 0;
  adminMenuLocked     = 0;
  adminMenuPin[0]     = 1;
  adminMenuPin[1]     = 1;
  adminMenuPin[2]     = 1;
  adminMenuPin[3]     = 1;

  writeSettingsToFlash();
}

void Settings::migrateSettings(int oldVersion) {
  if (oldVersion == 1) {
    Serial.println(F("=== resetSettings()"));
    Serial.println(F("1 -> 2"));
    version = 2;
    adminMenuLocked = 0;
    adminMenuPin[0] = 1;
    adminMenuPin[1] = 1;
    adminMenuPin[2] = 1;
    adminMenuPin[3] = 1;
    writeSettingsToFlash();
  }
}

void Settings::loadSettingsFromFlash() {
  Serial.println(F("=== loadSettingsFromFlash()"));
  EEPROM.get(startAddressAdminSettings, *this);
  if (cookie != cardCookie)
    resetSettings();
  migrateSettings(version);

  Serial.print(F("Version: "));
  Serial.println(version);

  Serial.print(F("Maximal Volume: "));
  Serial.println(maxVolume);

  Serial.print(F("Minimal Volume: "));
  Serial.println(minVolume);

  Serial.print(F("Initial Volume: "));
  Serial.println(initVolume);

  Serial.print(F("EQ: "));
  Serial.println(eq);

  Serial.print(F("Locked: "));
  Serial.println(locked);

  Serial.print(F("Sleep Timer: "));
  Serial.println(standbyTimer);

  Serial.print(F("Inverted Volume Buttons: "));
  Serial.println(invertVolumeButtons);

  Serial.print(F("Admin Menu locked: "));
  Serial.println(adminMenuLocked);

  Serial.print(F("Admin Menu Pin: "));
  Serial.print(adminMenuPin[0]);
  Serial.print(adminMenuPin[1]);
  Serial.print(adminMenuPin[2]);
  Serial.println(adminMenuPin[3]);
}

void Settings::writeFolderSettingToFlash(uint8_t folder, uint16_t track) {
  EEPROM.update(folder, track);
}

uint16_t Settings::readFolderSettingFromFlash(uint8_t folder) {
  return EEPROM.read(folder);
}


