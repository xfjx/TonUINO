#include "tonuino.hpp"

#include <Arduino.h>
#include <avr/sleep.h>

#include "array.hpp"
#include "chip_card.hpp"

Tonuino tonuino;

namespace {

const uint8_t shutdownPin   =  7;

}

void Tonuino::setup() {
  pinMode(shutdownPin  , OUTPUT);
  digitalWrite(shutdownPin, LOW);

  // load Settings from EEPROM
  settings.loadSettingsFromFlash();

  // activate standby timer
  tonuino.setStandbyTimer();

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
  tonuino.playShortCut(3);
}

void Tonuino::loop() {

  checkStandbyAtMillis();

  mp3.loop();

  // Modifier : WIP!
  activeModifier->loop();

  handleButtons();
  handleChipCard();

}

void Tonuino::handleButtons() {

  switch (buttons.getButton()) {

  case button::admin:
    mp3.pause();
    buttons.waitForNoButton();
    if (not adminMenuAllowed()) {
      mp3.start();
      break;
    }
    adminMenu();
    break;

  case button::pause:
    if (activeModifier->handlePause())
      break;
    if (mp3.isPlaying()) {
      mp3.pause();
      setStandbyTimer();
    } else if (knownCard) {
      mp3.start();
      disableStandbyTimer();
    }
    break;

  case button::track:
    if (activeModifier->handlePause())
      break;
    if (mp3.isPlaying()) {
      uint8_t advertTrack = getCurrentTrack();
      // Spezialmodus Von-Bis für Album und Party gibt die Dateinummer relativ zur Startposition wieder
      if (myFolder->mode == mode_t::album_vb || myFolder->mode == mode_t::party_vb) {
        advertTrack = advertTrack - myFolder->special + 1;
      }
      mp3.playAdvertisement(advertTrack);
    } else {
      playShortCut(0);
    }
    break;

  case button::volume_up:
    if (mp3.isPlaying())
      volumeUpButton();
    else
      playShortCut(1);
    break;

  case button::next:
    if (mp3.isPlaying())
      nextButton();
    else
      playShortCut(1);
    break;

  case button::volume_down:
    if (mp3.isPlaying())
      volumeDownButton();
    else
      playShortCut(2);
    break;

  case button::previous:
    if (mp3.isPlaying())
      previousButton();
    else
      playShortCut(2);
    break;
  default:
    break;
  }
}

void Tonuino::handleChipCard() {
  if (!chip_card.newCardPresent())
    return;

  // RFID Karte wurde aufgelegt
  nfcTagObject tempCard;
  if (chip_card.readCard(tempCard) && !specialCard(tempCard) && !activeModifier->handleRFID(tempCard)) {
    setCard(tempCard);
    Serial.println(myCard.nfcFolderSettings.folder);

    if (myCard.cookie == cardCookie && myCard.nfcFolderSettings.mode != mode_t::none) {
      knownCard = false; // prevent nextTrack() when calling Mp3Notify::OnPlayFinished() in mp3.waitForTrackToFinish();
      mp3.playMp3FolderTrack(mp3Tracks::t_262_pling);
      mp3.waitForTrackToFinish();
      playFolder();
    }

    // Neue Karte konfigurieren
    else {
      knownCard = false;
      mp3.playMp3FolderTrack(mp3Tracks::t_300_new_tag);
      mp3.waitForTrackToFinish();
      setupCard();
    }
  }
  chip_card.stopCard();
}

void Tonuino::playFolder() {
  Serial.println(F("== playFolder()"));
  disableStandbyTimer();
  knownCard = true;
  numTracksInFolder = mp3.getFolderTrackCount(myFolder->folder);
  firstTrack = 1;
  Serial.print(numTracksInFolder);
  Serial.print(F(" Dateien in Ordner "));
  Serial.println(myFolder->folder);

  switch (myFolder->mode) {

  case mode_t::hoerspiel:
    // Hörspielmodus: eine zufällige Datei aus dem Ordner
    Serial.println(F("Hörspielmodus -> zufälligen Track wiedergeben"));
    currentTrack = random(1, numTracksInFolder + 1);
    Serial.println(currentTrack);
    break;

  case mode_t::album:
    // Album Modus: kompletten Ordner spielen
    Serial.println(F("Album Modus -> kompletten Ordner wiedergeben"));
    currentTrack = 1;
    break;

  case mode_t::party:
    // Party Modus: Ordner in zufälliger Reihenfolge
    Serial.println(
        F("Party Modus -> Ordner in zufälliger Reihenfolge wiedergeben"));
    shuffleQueue();
    currentTrack = 1;
    break;

  case mode_t::einzel:
    // Einzel Modus: eine Datei aus dem Ordner abspielen
    Serial.println(F("Einzel Modus -> eine Datei aus dem Odrdner abspielen"));
    currentTrack = myFolder->special;
    break;

  case mode_t::hoerbuch:
  case mode_t::hoerbuch_1:
    // Hörbuch Modus: kompletten Ordner spielen und Fortschritt merken (oder nur eine Datei)
    Serial.println(F("Hörbuch Modus -> kompletten Ordner spielen und Fortschritt merken"));
    currentTrack = settings.readFolderSettingFromFlash(myFolder->folder);
    if (currentTrack == 0 || currentTrack > numTracksInFolder) {
      currentTrack = 1;
    }
    break;

  case mode_t::hoerspiel_vb:
    // Spezialmodus Von-Bin: Hörspiel: eine zufällige Datei aus dem Ordner
    Serial.println(F("Spezialmodus Von-Bin: Hörspiel -> zufälligen Track wiedergeben"));
    Serial.print(myFolder->special);
    Serial.print(F(" bis "));
    Serial.println(myFolder->special2);
    firstTrack = myFolder->special;
    numTracksInFolder = myFolder->special2;
    currentTrack = random(myFolder->special, numTracksInFolder + 1);
    Serial.println(currentTrack);
    break;

  case mode_t::album_vb:
    // Spezialmodus Von-Bis: Album: alle Dateien zwischen Start und Ende spielen
    Serial.println(F("Spezialmodus Von-Bis: Album: alle Dateien zwischen Start- und Enddatei spielen"));
    Serial.print(myFolder->special);
    Serial.print(F(" bis "));
    Serial.println(myFolder->special2);
    firstTrack = myFolder->special;
    numTracksInFolder = myFolder->special2;
    currentTrack = myFolder->special;
    break;

  case mode_t::party_vb:
    // Spezialmodus Von-Bis: Party Ordner in zufälliger Reihenfolge
    Serial.println(F("Spezialmodus Von-Bis: Party -> Ordner in zufälliger Reihenfolge wiedergeben"));
    firstTrack = myFolder->special;
    numTracksInFolder = myFolder->special2;
    shuffleQueue();
    currentTrack = 1;
    break;
  default:
    knownCard = false;
    setStandbyTimer();
    return;
  }
  playCurrentTrack();
}

void Tonuino::playShortCut(uint8_t shortCut) {
  Serial.println(F("=== playShortCut()"));
  Serial.println(shortCut);
  if (settings.shortCuts[shortCut].folder != 0) {
    setFolder(&settings.shortCuts[shortCut]);
    playFolder();
    delay(1000);
  } else {
    Serial.println(F("Shortcut not configured!"));
    mp3.playMp3FolderTrack(mp3Tracks::t_262_pling);
  }
}

// Leider kann das Modul selbst keine Queue abspielen, daher müssen wir selbst die Queue verwalten
void Tonuino::nextTrack() {
  if (activeModifier->handleNext())
    return;

  if (not knownCard)
    // Wenn eine neue Karte angelernt wird soll das Ende eines Tracks nicht
    // verarbeitet werden
    return;

  Serial.println(F("=== nextTrack()"));

  switch (myFolder->mode) {
  case mode_t::hoerspiel   :
  case mode_t::hoerspiel_vb:
    if (not mp3.isPlaying()) {
      Serial.println(F("Hörspielmodus ist aktiv -> keinen neuen Track spielen"));
      knownCard = false;
      setStandbyTimer();
      //mp3.sleep(); // Je nach Modul kommt es nicht mehr zurück aus dem Sleep!
    }
    break;

  case mode_t::album   :
  case mode_t::album_vb:
    if (currentTrack != numTracksInFolder) {
      ++currentTrack;
      mp3.playFolderTrack(myFolder->folder, currentTrack);
      Serial.print(F("Albummodus ist aktiv -> nächster Track: "));
      Serial.println(currentTrack);
    } else {
      //      mp3.sleep();   // Je nach Modul kommt es nicht mehr zurück aus dem Sleep!
      knownCard = false;
      setStandbyTimer();
    }
    break;

  case mode_t::party   :
  case mode_t::party_vb:
    if (currentTrack != numTracksInFolder - firstTrack + 1) {
      Serial.print(F("Party -> weiter in der Queue "));
      ++currentTrack;
    } else {
      Serial.println(F("Ende der Queue -> beginne von vorne"));
      currentTrack = 1;
      //// Wenn am Ende der Queue neu gemischt werden soll bitte die Zeilen wieder aktivieren
      //Serial.println(F("Ende der Queue -> mische neu"));
      //shuffleQueue();
    }
    Serial.println(queue[currentTrack - 1]);
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
    break;

  case mode_t::einzel:
    Serial.println(F("Einzel Modus aktiv -> Strom sparen"));
    //mp3.sleep();      // Je nach Modul kommt es nicht mehr zurück aus dem Sleep!
    knownCard = false;
    setStandbyTimer();
    break;

  case mode_t::hoerbuch:
    if (currentTrack != numTracksInFolder) {
      ++currentTrack;
      Serial.print(F("Hörbuch Modus ist aktiv -> nächster Track und Fortschritt speichern"));
      Serial.println(currentTrack);
      mp3.playFolderTrack(myFolder->folder, currentTrack);
      // Fortschritt im EEPROM abspeichern
      settings.writeFolderSettingToFlash(myFolder->folder, currentTrack);
    } else {
      //mp3.sleep();  // Je nach Modul kommt es nicht mehr zurück aus dem Sleep!
      // Fortschritt zurück setzen
      settings.writeFolderSettingToFlash(myFolder->folder, 1);
      knownCard = false;
      setStandbyTimer();
    }
    break;
  case mode_t::hoerbuch_1:
    if (currentTrack != numTracksInFolder)
      ++currentTrack;
    else
      currentTrack = 1;

    Serial.print(F("Hörbuch Modus single ist aktiv -> Fortschritt speichern und beenden"));
    Serial.println(currentTrack);
    // Fortschritt im EEPROM abspeichern
    settings.writeFolderSettingToFlash(myFolder->folder, currentTrack);
    //mp3.sleep();  // Je nach Modul kommt es nicht mehr zurück aus dem Sleep!
    knownCard = false;
    setStandbyTimer();
    break;
    default:
    break;
  }
  delay(500);
}

void Tonuino::previousTrack() {
  Serial.println(F("=== previousTrack()"));

  switch (myFolder->mode) {
  case mode_t::hoerspiel:
  case mode_t::hoerspiel_vb:
    Serial.println(F("Hörspielmodus ist aktiv -> Track von vorne spielen"));
    mp3.playFolderTrack(myFolder->folder, currentTrack);
    break;

  case mode_t::album:
  case mode_t::album_vb:
    Serial.println(F("Albummodus ist aktiv -> vorheriger Track"));
    if (currentTrack != firstTrack) {
      --currentTrack;
    }
    mp3.playFolderTrack(myFolder->folder, currentTrack);
    break;

  case mode_t::party:
  case mode_t::party_vb:
    if (currentTrack != 1) {
      Serial.print(F("Party Modus ist aktiv -> zurück in der Qeueue "));
      --currentTrack;
    } else {
      Serial.print(F("Anfang der Queue -> springe ans Ende "));
      currentTrack = numTracksInFolder - firstTrack + 1;
    }
    Serial.println(queue[currentTrack - 1]);
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
    break;

  case mode_t::einzel:
    Serial.println(F("Einzel Modus aktiv -> Track von vorne spielen"));
    mp3.playFolderTrack(myFolder->folder, currentTrack);
    break;

  case mode_t::hoerbuch:
  case mode_t::hoerbuch_1:
    Serial.println(F("Hörbuch Modus ist aktiv -> vorheriger Track und Fortschritt speichern"));
    if (currentTrack != 1) {
      --currentTrack;
    }
    mp3.playFolderTrack(myFolder->folder, currentTrack);
    // Fortschritt im EEPROM abspeichern
    settings.writeFolderSettingToFlash(myFolder->folder, currentTrack);
    break;
    default:
    break;
  }
  delay(500);
}

// Funktionen für den Standby Timer (z.B. über Pololu-Switch oder Mosfet)
void Tonuino::setStandbyTimer() {
  Serial.println(F("=== setStandbyTimer()"));
  if (settings.standbyTimer != 0)
    standbyAtMillis = millis() + (settings.standbyTimer * 60 * 1000);
  else
    standbyAtMillis = 0;
  Serial.println(standbyAtMillis);
}

void Tonuino::disableStandbyTimer() {
  Serial.println(F("=== disablestandby()"));
  standbyAtMillis = 0;
}

void Tonuino::checkStandbyAtMillis() {
  if (standbyAtMillis != 0 && millis() > standbyAtMillis) {
    Serial.println(F("=== power off!"));
    // enter sleep state
    digitalWrite(shutdownPin, HIGH);
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

void Tonuino::volumeUpButton() {
  if (activeModifier->handleVolumeUp())
    return;

  Serial.println(F("=== volumeUp()"));
  mp3.increaseVolume();
}

void Tonuino::volumeDownButton() {
  if (activeModifier->handleVolumeDown())
    return;

  Serial.println(F("=== volumeDown()"));
  mp3.decreaseVolume();
}

void Tonuino::nextButton() {
  if (activeModifier->handleNextButton())
    return;

  nextTrack();
}

void Tonuino::previousButton() {
  if (activeModifier->handlePreviousButton())
    return;

  previousTrack();
}

bool Tonuino::setupFolder(folderSettings& theFolder) {
  // Ordner abfragen
  theFolder.folder = voiceMenu(99, mp3Tracks::t_301_select_folder, mp3Tracks::t_0, true, 0, 0, true);
  if (theFolder.folder == 0) return false;

  // Wiedergabemodus abfragen
  theFolder.mode = static_cast<mode_t>(voiceMenu(10, mp3Tracks::t_310_select_mode, mp3Tracks::t_310_select_mode, false, 0, 0, true));
  if (theFolder.mode == mode_t::none) return false;

  //// Hörbuchmodus -> Fortschritt im EEPROM auf 1 setzen
  //writeFolderSettingToFlash(theFolder.folder, 1);

  switch (theFolder.mode) {

  // Einzelmodus -> Datei abfragen
  case mode_t::einzel:
    theFolder.special = voiceMenu(mp3.getFolderTrackCount(theFolder.folder), mp3Tracks::t_327_select_file, mp3Tracks::t_0,
                                  true, theFolder.folder);
    break;

  // Admin Funktionen
  case mode_t::admin:
    theFolder.folder = 0;
    theFolder.mode = mode_t::admin_card;
    break;

  // Spezialmodus Von-Bis
  case mode_t::hoerspiel_vb:
  case mode_t::album_vb:
  case mode_t::party_vb:
    theFolder.special  = voiceMenu(mp3.getFolderTrackCount(theFolder.folder), mp3Tracks::t_328_select_first_file, mp3Tracks::t_0,
                                   true, theFolder.folder);
    theFolder.special2 = voiceMenu(mp3.getFolderTrackCount(theFolder.folder), mp3Tracks::t_329_select_last_file, mp3Tracks::t_0,
                                   true, theFolder.folder, theFolder.special);
    break;

  default:
    break;
  }
  return true;
}

void Tonuino::resetCard() {
  mp3.playMp3FolderTrack(mp3Tracks::t_800_waiting_for_card);
  do {
    if (buttons.isBreak()) {
      mp3.playMp3FolderTrack(mp3Tracks::t_802_reset_aborted);
      return;
    }
  } while (!chip_card.newCardPresent());

  Serial.print(F("Karte wird neu konfiguriert!"));
  setupCard();
}

void Tonuino::setupCard() {
  mp3.pause();
  Serial.println(F("=== setupCard()"));
  nfcTagObject newCard;
  if (setupFolder(newCard.nfcFolderSettings))
  {
    // Karte ist konfiguriert -> speichern
    mp3.pause();
    do {
    } while (mp3.isPlaying());
    if (chip_card.writeCard(newCard))
      mp3.playMp3FolderTrack(mp3Tracks::t_400_ok);
    else
      mp3.playMp3FolderTrack(mp3Tracks::t_401_error);
    mp3.waitForTrackToFinish();
  }
}

bool Tonuino::specialCard(const nfcTagObject &nfcTag) {
  if (nfcTag.cookie != cardCookie)
    return false;

  if (nfcTag.nfcFolderSettings.folder != 0)
    return false;

  //Serial.print(F("special card, mode = "));
  //Serial.println(static_cast<uint8_t>(nfcTag.nfcFolderSettings.mode));
  if (activeModifier->getActive() == nfcTag.nfcFolderSettings.mode) {
    resetActiveModifier();
    Serial.println(F("modifier removed"));
    mp3.playAdvertisement(advertTracks::t_261_deactivate_mod_card, false);
    return true;
  }
  const Modifier *oldModifier = activeModifier;

  switch (nfcTag.nfcFolderSettings.mode) {
  case mode_t::none:
  case mode_t::admin_card:   chip_card.stopCard();
                             adminMenu()                                                      ;break;
  case mode_t::sleep_timer:  Serial.println(F("activate sleepTimer"));
                             mp3.playAdvertisement(advertTracks::t_302_sleep, false);
                             activeModifier = &sleepTimer;
                             sleepTimer.start(nfcTag.nfcFolderSettings.special)               ;break;
  case mode_t::freeze_dance: Serial.println(F("activate freezeDance"));
                             mp3.playAdvertisement(advertTracks::t_300_freeze_into, false);
                             activeModifier = &freezeDance;                                   ;break;
  case mode_t::locked:       Serial.println(F("activate locked"));
                             mp3.playAdvertisement(advertTracks::t_303_locked, false);
                             activeModifier = &locked                                         ;break;
  case mode_t::toddler:      Serial.println(F("activate toddlerMode"));
                             mp3.playAdvertisement(advertTracks::t_304_buttonslocked, false);
                             activeModifier = &toddlerMode                                    ;break;
  case mode_t::kindergarden: Serial.println(F("activate kindergardenMode"));
                             mp3.playAdvertisement(advertTracks::t_305_kindergarden, false);
                             activeModifier = &kindergardenMode                               ;break;
  case mode_t::repeat_single:Serial.println(F("activate repeatSingleModifier"));
                             mp3.playAdvertisement(advertTracks::t_260_activate_mod_card, false);
                             activeModifier = &repeatSingleModifier                           ;break;
  default:                                                                                     break;
  }
  if (oldModifier != activeModifier)
    activeModifier->init();
  delay(2000);
  return true;
}

bool Tonuino::adminMenuAllowed() {
  // Admin menu has been locked - it still can be trigged via admin card
  switch (settings.adminMenuLocked) {
  case 1:
    return false;

  // Pin check
  case 2:
    Settings::pin_t pin;
    mp3.playMp3FolderTrack(mp3Tracks::t_991_admin_pin);
    return (buttons.askCode(pin) && pin == settings.adminMenuPin);

  // Match check
  case 3:
  {
    const uint8_t a = random(10, 20);
    const uint8_t b = random(1, a);
    uint8_t c;
    mp3.playMp3FolderTrack(mp3Tracks::t_992_admin_calc);
    mp3.waitForTrackToFinish();
    mp3.playMp3FolderTrack(a);
    mp3.waitForTrackToFinish();

    if (random(1, 3) == 2) {
      // a + b
      c = a + b;
      mp3.playMp3FolderTrack(mp3Tracks::t_993_admin_calc);
    } else {
      // a - b
      c = a - b;
      mp3.playMp3FolderTrack(mp3Tracks::t_994_admin_calc);
    }
    mp3.waitForTrackToFinish();
    mp3.playMp3FolderTrack(b);
    Serial.println(c);
    return (voiceMenu(255, mp3Tracks::t_0, mp3Tracks::t_0, false) == c);
  }
  }

  // not locked
  return true;
}

void Tonuino::adminMenu() {
  disableStandbyTimer();
  mp3.pause();
  Serial.println(F("=== adminMenu()"));
  knownCard = false;

  const int subMenu = voiceMenu(12, mp3Tracks::t_900_admin, mp3Tracks::t_900_admin, false, false, 0, true);

  switch (subMenu) {
  case 0:  setStandbyTimer();
           return;
  case 1:  resetCard();
           chip_card.stopCard();
           break;
  case 2:  // Maximum Volume
           settings.maxVolume = voiceMenu(30 - settings.minVolume, mp3Tracks::t_930_max_volume_intro, static_cast<mp3Tracks>(settings.minVolume), false, false, settings.maxVolume - settings.minVolume) + settings.minVolume;
           break;
  case 3:  // Minimum Volume
           settings.minVolume = voiceMenu(settings.maxVolume - 1, mp3Tracks::t_931_min_volume_into, mp3Tracks::t_0, false, false, settings.minVolume);
           break;
  case 4:  // Initial Volume
           settings.initVolume = voiceMenu(settings.maxVolume - settings.minVolume + 1, mp3Tracks::t_932_init_volume_into, static_cast<mp3Tracks>(settings.minVolume - 1), false, false, settings.initVolume - settings.minVolume + 1) + settings.minVolume - 1;
           break;
  case 5:  // EQ
           settings.eq = voiceMenu(6, mp3Tracks::t_920_eq_intro, mp3Tracks::t_920_eq_intro, false, false, settings.eq);
           mp3.setEq(static_cast<DfMp3_Eq>(settings.eq - 1));
           break;
  case 6:  // create modifier card
           createModifierCard();
           break;
  case 7:  // shortcut
           setupFolder(settings.shortCuts[voiceMenu(4, mp3Tracks::t_940_shortcut_into, mp3Tracks::t_940_shortcut_into) - 1]);
           mp3.playMp3FolderTrack(mp3Tracks::t_400_ok);
           break;
  case 8:  // standby timer
           switch (voiceMenu(5, mp3Tracks::t_960_timer_intro, mp3Tracks::t_960_timer_intro)) {
           case 1: settings.standbyTimer =  5; break;
           case 2: settings.standbyTimer = 15; break;
           case 3: settings.standbyTimer = 30; break;
           case 4: settings.standbyTimer = 60; break;
           case 5: settings.standbyTimer =  0; break;
           }
           break;
  case 9:  // Create Cards for Folder
           createCardsForFolder();
           break;
  case 10: // Invert Functions for Up/Down Buttons
           if (voiceMenu(2, mp3Tracks::t_933_switch_volume_intro, mp3Tracks::t_933_switch_volume_intro, false) == 2) {
             settings.invertVolumeButtons = true;
           }
           else {
             settings.invertVolumeButtons = false;
           }
           break;
  case 11: // reset EEPROM
           settings.clearEEPROM();
           settings.resetSettings();
           mp3.playMp3FolderTrack(mp3Tracks::t_999_reset_ok);
           break;
  case 12: // lock admin menu
           switch (voiceMenu(4, mp3Tracks::t_980_admin_lock_intro, mp3Tracks::t_980_admin_lock_intro, false)) {
           case 1: settings.adminMenuLocked = 0;
                   break;
           case 2: settings.adminMenuLocked = 1;
                   break;
           case 3: {
                     Settings::pin_t pin;
                     mp3.playMp3FolderTrack(mp3Tracks::t_991_admin_pin);
                     if (buttons.askCode(pin))
                       settings.adminMenuPin = pin;
                     else
                       break;
                   }
                   settings.adminMenuLocked = 2;
                   break;
           }
           break;
  }
  settings.writeSettingsToFlash();
  mp3.playMp3FolderTrack(mp3Tracks::t_262_pling);
  setStandbyTimer();
}

void Tonuino::voiceMenuPlayOption( uint8_t   returnValue
                                 , mp3Tracks messageOffset
                                 , bool      preview
                                 , int       previewFromFolder) {
  Serial.println(returnValue);
  //mp3.pause();
  mp3.playMp3FolderTrack(messageOffset + returnValue);
  if (preview) {
    mp3.waitForTrackToFinish();
    if (previewFromFolder == 0)
      mp3.playFolderTrack(returnValue, 1);
    else
      mp3.playFolderTrack(previewFromFolder, returnValue);
  }
}

uint8_t Tonuino::voiceMenu( int       numberOfOptions
                          , mp3Tracks startMessage
                          , mp3Tracks messageOffset
                          , bool      preview
                          , int       previewFromFolder
                          , int       defaultValue
                          , bool      exitWithLongPress) {
  uint8_t returnValue = defaultValue;

  if (startMessage != mp3Tracks::t_0)
    mp3.playMp3FolderTrack(startMessage);

  Serial.print(F("=== voiceMenu() ("));
  Serial.print(numberOfOptions);
  Serial.println(F(" Options)"));

  do {
    if (Serial.available() > 0) {
      int optionSerial = Serial.parseInt();
      if (optionSerial != 0 && optionSerial <= numberOfOptions)
        return optionSerial;
    }
    mp3.loop();
    switch(buttons.getButton()) {
    case button::track:
      if (exitWithLongPress) {
        mp3.playMp3FolderTrack(mp3Tracks::t_802_reset_aborted);
        return defaultValue;
      }
      break;

    case button::pause:
      if (returnValue != 0) {
        Serial.print(F("=== "));
        Serial.print(returnValue);
        Serial.println(F(" ==="));
        return returnValue;
      }
      //delay(1000);
      break;

    case button::next:
      returnValue = min(returnValue + 10, numberOfOptions);
      voiceMenuPlayOption(returnValue, messageOffset, preview, previewFromFolder);
      break;

    case button::volume_up:
      returnValue = min(returnValue + 1, numberOfOptions);
      voiceMenuPlayOption(returnValue, messageOffset, preview, previewFromFolder);
      break;

    case button::previous:
      returnValue = max(returnValue - 10, 1);
      voiceMenuPlayOption(returnValue, messageOffset, preview, previewFromFolder);
      break;

    case button::volume_down:
      returnValue = max(returnValue - 1, 1);
      voiceMenuPlayOption(returnValue, messageOffset, preview, previewFromFolder);
      break;
    default:
      break;
    }
  } while (true);
}

void Tonuino::createModifierCard() {
  nfcTagObject tempCard;
  tempCard.cookie = cardCookie;
  tempCard.version = 1;
  tempCard.nfcFolderSettings.folder = 0;
  tempCard.nfcFolderSettings.special = 0;
  tempCard.nfcFolderSettings.special2 = 0;
  tempCard.nfcFolderSettings.mode = static_cast<mode_t>(voiceMenu(6, mp3Tracks::t_970_modifier_Intro, mp3Tracks::t_970_modifier_Intro, false, false, 0, true));

  if (tempCard.nfcFolderSettings.mode != mode_t::none) {
    if (tempCard.nfcFolderSettings.mode == mode_t::sleep_timer) {
      switch (voiceMenu(4, mp3Tracks::t_960_timer_intro, mp3Tracks::t_960_timer_intro)) {
        case 1: tempCard.nfcFolderSettings.special =  5; break;
        case 2: tempCard.nfcFolderSettings.special = 15; break;
        case 3: tempCard.nfcFolderSettings.special = 30; break;
        case 4: tempCard.nfcFolderSettings.special = 60; break;
      }
    }
    mp3.playMp3FolderTrack(mp3Tracks::t_800_waiting_for_card);
    do {
      if (buttons.isBreak()) {
        mp3.playMp3FolderTrack(mp3Tracks::t_802_reset_aborted);
        return;
      }
    } while (!chip_card.newCardPresent());

    // RFID Karte wurde aufgelegt
    Serial.println(F("schreibe Karte..."));
    if (chip_card.writeCard(tempCard))
      mp3.playMp3FolderTrack(mp3Tracks::t_400_ok);
    else
      mp3.playMp3FolderTrack(mp3Tracks::t_401_error);
    mp3.waitForTrackToFinish();
    chip_card.stopCard();
  }
}

void Tonuino::createCardsForFolder() {
  // Ordner abfragen
  nfcTagObject tempCard;
  tempCard.cookie = cardCookie;
  tempCard.version = 1;
  tempCard.nfcFolderSettings.mode = mode_t::einzel;
  tempCard.nfcFolderSettings.folder = voiceMenu(99, mp3Tracks::t_301_select_folder, mp3Tracks::t_0, true);
  uint8_t special = voiceMenu(mp3.getFolderTrackCount(tempCard.nfcFolderSettings.folder), mp3Tracks::t_328_select_first_file, mp3Tracks::t_0,
                              true, tempCard.nfcFolderSettings.folder);
  uint8_t special2 = voiceMenu(mp3.getFolderTrackCount(tempCard.nfcFolderSettings.folder), mp3Tracks::t_329_select_last_file, mp3Tracks::t_0,
                               true, tempCard.nfcFolderSettings.folder, special);

  mp3.playMp3FolderTrack(mp3Tracks::t_936_batch_cards_intro);
  mp3.waitForTrackToFinish();
  for (uint8_t x = special; x <= special2; x++) {
    mp3.playMp3FolderTrack(x);
    tempCard.nfcFolderSettings.special = x;
    Serial.print(x);
    Serial.println(F(" Karte auflegen"));
    do {
      if (buttons.isBreak()) {
        mp3.playMp3FolderTrack(mp3Tracks::t_802_reset_aborted);
        return;
      }
    } while (!chip_card.newCardPresent());

    // RFID Karte wurde aufgelegt
    Serial.println(F("schreibe Karte..."));
    if (chip_card.writeCard(tempCard))
      mp3.playMp3FolderTrack(mp3Tracks::t_400_ok);
    else
      mp3.playMp3FolderTrack(mp3Tracks::t_401_error);
    mp3.waitForTrackToFinish();
    chip_card.stopCard();
    mp3.waitForTrackToFinish();
  }
}

void Tonuino::shuffleQueue() {
  // Queue für die Zufallswiedergabe erstellen
  for (uint8_t x = 0; x < numTracksInFolder - firstTrack + 1; x++)
    queue[x] = x + firstTrack;

  // Rest mit 0 auffüllen
  for (uint8_t x = numTracksInFolder - firstTrack + 1; x < maxTracksInFolder; x++)
    queue[x] = 0;

  // Queue mischen
  for (uint8_t i = 0; i < numTracksInFolder - firstTrack + 1; i++) {
    const uint8_t j = random(0, numTracksInFolder - firstTrack + 1);
    const uint8_t t = queue[i];
    queue[i]        = queue[j];
    queue[j]        = t;
  }
//  Serial.println(F("Queue :"));
//  for (uint8_t x = 0; x < numTracksInFolder - firstTrack + 1 ; x++)
//  Serial.println(queue[x]);
}



