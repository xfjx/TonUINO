#include "src/tonuino.hpp"

#include "src/settings.hpp"
#include "src/mp3.hpp"
#include "src/buttons.hpp"
#include "src/logger.hpp"
#include "src/constants.hpp"


/*
   _____         _____ _____ _____ _____
  |_   _|___ ___|  |  |     |   | |     |
    | | | . |   |  |  |-   -| | | |  |  |
    |_| |___|_|_|_____|_____|_|___|_____|
    TonUINO Version 2.1

    created by Thorsten Voß and licensed under GNU/GPL.
    refactored by Boerge1
    Information and contribution at https://tonuino.de.
*/

void setup() {

  Serial.begin(115200); // Es gibt ein paar Debug Ausgaben über die serielle Schnittstelle

  // Wert für randomSeed() erzeugen durch das mehrfache Sammeln von rauschenden LSBs eines offenen Analogeingangs
  uint32_t ADC_LSB = 0;
  uint32_t ADCSeed = 0;
  for (uint8_t i = 0; i < 128; i++) {
    ADC_LSB = analogRead(openAnalogPin) & 0x1;
    ADCSeed ^= ADC_LSB << (i % 32);
  }
  randomSeed(ADCSeed); // Zufallsgenerator initialisieren

  // Dieser Hinweis darf nicht entfernt werden
  LOG(init_log, s_debug, F(" _____         _____ _____ _____ _____"));
  LOG(init_log, s_debug, F("|_   _|___ ___|  |  |     |   | |     |"));
  LOG(init_log, s_debug, F("  | | | . |   |  |  |-   -| | | |  |  |"));
  LOG(init_log, s_debug, F("  |_| |___|_|_|_____|_____|_|___|_____|\n"));
  LOG(init_log, s_debug, F("TonUINO Version 3.0"));
  LOG(init_log, s_debug, F("created by Thorsten Voß and licensed under GNU/GPL."));
  LOG(init_log, s_debug, F("refactored by Boerge1."));
  LOG(init_log, s_debug, F("Information and contribution at https://tonuino.de.\n"));

  Tonuino::getTonuino().setup();
}

void loop() {

  Tonuino::getTonuino().loop();
}
