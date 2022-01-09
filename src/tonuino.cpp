#include "tonuino.hpp"

#include <Arduino.h>
#include <avr/sleep.h>

#include "array.hpp"
#include "chip_card.hpp"

#include "constants.hpp"
#include "logger.hpp"
#include "state_machine.hpp"

namespace {

template<typename T>
void swap(T &lhs, T &rhs) {
  const T t = lhs;
  lhs       = rhs;
  rhs       = t;
}

const __FlashStringHelper* str_Track    () { return F("Track: ") ; }
const __FlashStringHelper* str_Hoerspiel() { return F("Hörspiel"); }
const __FlashStringHelper* str_Album    () { return F("Album"); }
const __FlashStringHelper* str_Party    () { return F("Party"); }
const __FlashStringHelper* str_Einzel   () { return F("Einzel"); }
const __FlashStringHelper* str_Hoerbuch () { return F("Hörbuch"); }
const __FlashStringHelper* str_Von_Bis  () { return F("Von-Bis "); }
const __FlashStringHelper* str_bis      () { return F(" bis "); }

} // anonymous namespace

void Tonuino::setup() {
  pinMode(shutdownPin  , OUTPUT);
  digitalWrite(shutdownPin, getLevel(shutdownPinType, level::inactive));

  // load Settings from EEPROM
  settings.loadSettingsFromFlash();

  // DFPlayer Mini initialisieren
  mp3.begin();
  // Zwei Sekunden warten bis der DFPlayer Mini initialisiert ist
  delay(2000);
  mp3.setVolume();
  mp3.setEq(static_cast<DfMp3_Eq>(settings.eq - 1));

  // NFC Leser initialisieren
  chip_card.initCard();

  // RESET --- ALLE DREI KNÖPFE BEIM STARTEN GEDRÜCKT HALTEN -> alle EINSTELLUNGEN werden gelöscht
  if (buttons.isReset()) {
    settings.clearEEPROM();
    settings.loadSettingsFromFlash();
  }

  // Start Shortcut "at Startup" - e.g. Welcome Sound
  //playShortCut(3);

  SM_tonuino::start();
  SM_tonuino::dispatch(button_e(buttonRaw::start));
}

void Tonuino::loop() {

  unsigned long  start = millis();
  checkStandbyAtMillis();

  mp3.loop();

  // Modifier : WIP!
  activeModifier->loop();

  SM_tonuino::dispatch(button_e(buttons.getButtonRaw()));
  SM_tonuino::dispatch(card_e(chip_card.getCardEvent()));

  unsigned long  stop = millis();

  if (stop-start < cycleTime)
    delay(cycleTime - (stop - start));
}

void Tonuino::playFolder() {
  LOG(play_log, s_debug, F("= playFolder()"));
  knownCard = true;
  numTracksInFolder = mp3.getFolderTrackCount(myFolder->folder);
  firstTrack = 1;
  LOG(play_log, s_info, numTracksInFolder, F(" files in folder "), myFolder->folder);

  switch (myFolder->mode) {

  case mode_t::hoerspiel:
    // Hörspielmodus: eine zufällige Datei aus dem Ordner
    LOG(play_log, s_info, str_Hoerspiel());
    currentTrack = random(1, numTracksInFolder + 1);
    LOG(play_log, s_info, str_Track(), currentTrack);
    break;

  case mode_t::album:
    // Album Modus: kompletten Ordner spielen
    LOG(play_log, s_info, str_Album());
    currentTrack = 1;
    break;

  case mode_t::party:
    // Party Modus: Ordner in zufälliger Reihenfolge
    LOG(play_log, s_info, str_Party());
    shuffleQueue();
    currentTrack = 1;
    break;

  case mode_t::einzel:
    // Einzel Modus: eine Datei aus dem Ordner abspielen
    LOG(play_log, s_info, str_Einzel());
    currentTrack = myFolder->special;
    LOG(play_log, s_info, str_Track(), currentTrack);
    break;

  case mode_t::hoerbuch:
  case mode_t::hoerbuch_1:
    // Hörbuch Modus: kompletten Ordner spielen und Fortschritt merken (oder nur eine Datei)
    LOG(play_log, s_info, str_Hoerbuch());
    currentTrack = settings.readFolderSettingFromFlash(myFolder->folder);
    if (currentTrack == 0 || currentTrack > numTracksInFolder) {
      currentTrack = 1;
    }
    LOG(play_log, s_info, str_Track(), currentTrack);
    break;

  case mode_t::hoerspiel_vb:
    // Spezialmodus Von-Bin: Hörspiel: eine zufällige Datei aus dem Ordner
    LOG(play_log, s_info, str_Von_Bis(), str_Hoerspiel());
    LOG(play_log, s_info, myFolder->special, F(" bis "), myFolder->special2);
    firstTrack = myFolder->special;
    numTracksInFolder = myFolder->special2;
    currentTrack = random(myFolder->special, numTracksInFolder + 1);
    LOG(play_log, s_info, str_Track(), currentTrack);
    break;

  case mode_t::album_vb:
    // Spezialmodus Von-Bis: Album: alle Dateien zwischen Start und Ende spielen
    LOG(play_log, s_info, str_Von_Bis(), str_Album());
    LOG(play_log, s_info, myFolder->special, str_bis() , myFolder->special2);
    firstTrack = myFolder->special;
    numTracksInFolder = myFolder->special2;
    currentTrack = myFolder->special;
    break;

  case mode_t::party_vb:
    // Spezialmodus Von-Bis: Party Ordner in zufälliger Reihenfolge
    LOG(play_log, s_info, str_Von_Bis(), str_Party());
    LOG(play_log, s_info, myFolder->special, str_bis(), myFolder->special2);
    firstTrack = myFolder->special;
    numTracksInFolder = myFolder->special2;
    shuffleQueue();
    currentTrack = 1;
    break;
  default:
    knownCard = false;
    return;
  }
  playCurrentTrack();
  if (knownCard && settings.pauseWhenCardRemoved)
    mp3.waitForTrackToStart(); // TODO remove waitForTrackToStart
}

void Tonuino::playTrackNumber () {
  uint8_t advertTrack = getCurrentTrack();
  // Spezialmodus Von-Bis für Album und Party gibt die Dateinummer relativ zur Startposition wieder
  if (myFolder->mode == mode_t::album_vb || myFolder->mode == mode_t::party_vb) {
    advertTrack = advertTrack - myFolder->special + 1;
  }
  mp3.playAdvertisement(advertTrack);
}


// Leider kann das Modul selbst keine Queue abspielen, daher müssen wir selbst die Queue verwalten
void Tonuino::nextTrack() {
  if (activeModifier->handleNext())
    return;

  if (not knownCard)
    // Wenn eine neue Karte angelernt wird soll das Ende eines Tracks nicht
    // verarbeitet werden
    return;

  LOG(play_log, s_info, F("= nextTrack()"));

  switch (myFolder->mode) {
  case mode_t::hoerspiel   :
  case mode_t::hoerspiel_vb:
    if (not mp3.isPlaying()) {
      LOG(play_log, s_info, str_Hoerspiel(), F(" -> stop"));
      knownCard = false;
      //mp3.sleep(); // Je nach Modul kommt es nicht mehr zurück aus dem Sleep!
    }
    break;

  case mode_t::album   :
  case mode_t::album_vb:
    if (currentTrack != numTracksInFolder) {
      ++currentTrack;
      mp3.playFolderTrack(myFolder->folder, currentTrack);
      LOG(play_log, s_info, str_Album(), F(" -> nächster Track: "), currentTrack);
    } else {
      //      mp3.sleep();   // Je nach Modul kommt es nicht mehr zurück aus dem Sleep!
      knownCard = false;
    }
    break;

  case mode_t::party   :
  case mode_t::party_vb:
    if (currentTrack != numTracksInFolder - firstTrack + 1) {
      LOG(play_log, s_info, str_Party(), F(" -> weiter in der Queue "));
      ++currentTrack;
    } else {
      LOG(play_log, s_info, str_Party(), F(", Ende der Queue -> beginne von vorne"));
      currentTrack = 1;
      //// Wenn am Ende der Queue neu gemischt werden soll bitte die Zeilen wieder aktivieren
      //LOG(play_log, s_info, F("Ende der Queue -> mische neu"));
      //shuffleQueue();
    }
    LOG(play_log, s_info, str_Track(), queue[currentTrack - 1]);
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
    break;

  case mode_t::einzel:
    LOG(play_log, s_info, str_Einzel(), F(" -> stop"));
    //mp3.sleep();      // Je nach Modul kommt es nicht mehr zurück aus dem Sleep!
    knownCard = false;
    break;

  case mode_t::hoerbuch:
    if (currentTrack != numTracksInFolder) {
      ++currentTrack;
      LOG(play_log, s_info, str_Hoerbuch(), F(" -> nächster Track und Fortschr. speichern"));
      LOG(play_log, s_info, str_Track(), currentTrack);
      mp3.playFolderTrack(myFolder->folder, currentTrack);
      // Fortschritt im EEPROM abspeichern
      settings.writeFolderSettingToFlash(myFolder->folder, currentTrack);
    } else {
      LOG(play_log, s_info, str_Hoerbuch(), F(" -> Ende"));
      //mp3.sleep();  // Je nach Modul kommt es nicht mehr zurück aus dem Sleep!
      // Fortschritt zurück setzen
      settings.writeFolderSettingToFlash(myFolder->folder, 1);
      knownCard = false;
    }
    break;
  case mode_t::hoerbuch_1:
    if (currentTrack != numTracksInFolder)
      ++currentTrack;
    else
      currentTrack = 1;

    LOG(play_log, s_info, str_Hoerbuch(), F(" single -> Fortschritt speichern und beenden"));
    LOG(play_log, s_info, str_Track(), currentTrack);
    // Fortschritt im EEPROM abspeichern
    settings.writeFolderSettingToFlash(myFolder->folder, currentTrack);
    //mp3.sleep();  // Je nach Modul kommt es nicht mehr zurück aus dem Sleep!
    knownCard = false;
    break;
    default:
    break;
  }
  delay(500); // TODO remove delay
}

void Tonuino::previousTrack() {
  LOG(play_log, s_info, F("= previousTrack()"));

  switch (myFolder->mode) {
  case mode_t::hoerspiel:
  case mode_t::hoerspiel_vb:
    LOG(play_log, s_info, str_Hoerspiel(), F(" -> Track von vorne spielen"));
    mp3.playFolderTrack(myFolder->folder, currentTrack);
    break;

  case mode_t::album:
  case mode_t::album_vb:
    LOG(play_log, s_info, str_Album(), F(" -> vorheriger Track"));
    if (currentTrack != firstTrack) {
      --currentTrack;
    }
    mp3.playFolderTrack(myFolder->folder, currentTrack);
    break;

  case mode_t::party:
  case mode_t::party_vb:
    if (currentTrack != 1) {
      LOG(play_log, s_info, str_Party(), F(" -> zurück in der Qeueue "));
      --currentTrack;
    } else {
      LOG(play_log, s_info, str_Party(), F(", Anfang der Queue -> springe ans Ende "));
      currentTrack = numTracksInFolder - firstTrack + 1;
    }
    LOG(play_log, s_info, str_Track(), queue[currentTrack - 1]);
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
    break;

  case mode_t::einzel:
    LOG(play_log, s_info, str_Einzel(), F(" -> Track von vorne spielen"));
    mp3.playFolderTrack(myFolder->folder, currentTrack);
    break;

  case mode_t::hoerbuch:
  case mode_t::hoerbuch_1:
    LOG(play_log, s_info, str_Hoerbuch(), F(" -> vorheriger Track und Fortschr. speichern"));
    if (currentTrack != 1) {
      --currentTrack;
    }
    LOG(play_log, s_info, str_Track(), currentTrack);
    mp3.playFolderTrack(myFolder->folder, currentTrack);
    // Fortschritt im EEPROM abspeichern
    settings.writeFolderSettingToFlash(myFolder->folder, currentTrack);
    break;
    default:
    break;
  }
  delay(500); // TODO remove delay
}

// Funktionen für den Standby Timer (z.B. über Pololu-Switch oder Mosfet)
void Tonuino::setStandbyTimer() {
  LOG(standby_log, s_info, F("= setStandbyTimer()"));
  if (settings.standbyTimer != 0 && not standbyTimer.isActive()) {
    standbyTimer.start(settings.standbyTimer * 60 * 1000);
    LOG(standby_log, s_info, F("timer started"));
  }
}

void Tonuino::disableStandbyTimer() {
  LOG(standby_log, s_info, F("= disablestandby()"));
  if (settings.standbyTimer != 0) {
    standbyTimer.stop();
    LOG(standby_log, s_info, F("timer stopped"));
  }
}

void Tonuino::checkStandbyAtMillis() {
  if (standbyTimer.isActive() && standbyTimer.isExpired()) {
    LOG(standby_log, s_info, F("power off!"));
    // enter sleep state
    digitalWrite(shutdownPin, getLevel(shutdownPinType, level::active));
    delay(500);

    // http://discourse.voss.earth/t/intenso-s10000-powerbank-automatische-abschaltung-software-only/805
    // powerdown to 27mA (powerbank switches off after 30-60s)
    chip_card.sleepCard();
    mp3.sleep();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();  // Disable interrupts
    sleep_mode();
  }
}

uint8_t Tonuino::getCurrentTrack() const {
  if (myFolder->mode == mode_t::party || myFolder->mode == mode_t::party_vb)
    return (queue[currentTrack - 1]);
  else
    return currentTrack;
}

bool Tonuino::specialCard(const nfcTagObject &nfcTag) {
  LOG(card_log, s_debug, F("special card, mode = "), static_cast<uint8_t>(nfcTag.nfcFolderSettings.mode));
  if (activeModifier->getActive() == nfcTag.nfcFolderSettings.mode) {
    resetActiveModifier();
    LOG(card_log, s_info, F("modifier removed"));
    mp3.playAdvertisement(advertTracks::t_261_deactivate_mod_card, false);
    return true;
  }
  const Modifier *oldModifier = activeModifier;

  switch (nfcTag.nfcFolderSettings.mode) {
  case mode_t::sleep_timer:  LOG(card_log, s_info, F("act. sleepTimer"));
                             mp3.playAdvertisement(advertTracks::t_302_sleep, false);
                             activeModifier = &sleepTimer;
                             sleepTimer.start(nfcTag.nfcFolderSettings.special)               ;break;
  case mode_t::freeze_dance: LOG(card_log, s_info, F("act. freezeDance"));
                             mp3.playAdvertisement(advertTracks::t_300_freeze_into, false);
                             activeModifier = &freezeDance;                                   ;break;
  case mode_t::locked:       LOG(card_log, s_info, F("act. locked"));
                             mp3.playAdvertisement(advertTracks::t_303_locked, false);
                             activeModifier = &locked                                         ;break;
  case mode_t::toddler:      LOG(card_log, s_info, F("act. toddlerMode"));
                             mp3.playAdvertisement(advertTracks::t_304_buttonslocked, false);
                             activeModifier = &toddlerMode                                    ;break;
  case mode_t::kindergarden: LOG(card_log, s_info, F("act. kindergardenMode"));
                             mp3.playAdvertisement(advertTracks::t_305_kindergarden, false);
                             activeModifier = &kindergardenMode                               ;break;
  case mode_t::repeat_single:LOG(card_log, s_info, F("act. repeatSingleModifier"));
                             mp3.playAdvertisement(advertTracks::t_260_activate_mod_card, false);
                             activeModifier = &repeatSingleModifier                           ;break;
  default:                   return false;
  }
  if (oldModifier != activeModifier)
    activeModifier->init();
  delay(2000);  // TODO remove delay
  return true;
}

void Tonuino::shuffleQueue() {
  // Queue für die Zufallswiedergabe erstellen
  for (uint8_t x = 0; x < numTracksInFolder - firstTrack + 1; ++x)
    queue[x] = x + firstTrack;

  // Rest mit 0 auffüllen
  for (uint8_t x = numTracksInFolder - firstTrack + 1; x < maxTracksInFolder; ++x)
    queue[x] = 0;

  // Queue mischen
  for (uint8_t i = 0; i < numTracksInFolder - firstTrack + 1; ++i) {
    const uint8_t j = random(0, numTracksInFolder - firstTrack + 1);
    swap(queue[i], queue[j]);
  }
//  LOG(menu_log, s_info, F("Queue :"));
//  for (uint8_t x = 0; x < numTracksInFolder - firstTrack + 1 ; ++x)
//  LOG(menu_log, s_info, queue[x]);
}



