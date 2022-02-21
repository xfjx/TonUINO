#ifndef SRC_MODIFIER_HPP_
#define SRC_MODIFIER_HPP_

#include <Arduino.h>

#include "chip_card.hpp"
#include "logger.hpp"
#include "timer.hpp"

class Tonuino;
class Mp3;
class Settings;
struct nfcTagObject;

class Modifier {
public:
  Modifier(Tonuino &tonuino, Mp3 &mp3, const Settings &settings): tonuino(tonuino), mp3(mp3), settings(settings) {}
  virtual void loop                () {}
  virtual bool handlePause         () { return false; }
  virtual bool handleNext          () { return false; }
  virtual bool handlePrevious      () { return false; }
  virtual bool handleNextButton    () { return false; }
  virtual bool handlePreviousButton() { return false; }
  virtual bool handleVolumeUp      () { return false; }
  virtual bool handleVolumeDown    () { return false; }
  virtual bool handleRFID(const nfcTagObject&)
                                      { return false; }
  virtual mode_t getActive         () { return mode_t::none; }
  virtual void init                () {}
  Modifier& operator=(const Modifier&) = delete;
protected:
  Tonuino        &tonuino;
  Mp3            &mp3;
  const Settings &settings;
};

class SleepTimer: public Modifier {
public:
  SleepTimer(Tonuino &tonuino, Mp3 &mp3, const Settings &settings): Modifier(tonuino, mp3, settings) {}
  void   loop     () final;
  mode_t getActive() final { return mode_t::sleep_timer; }
  void   start    (uint8_t minutes);

private:
  Timer sleepTimer{};
};

class FreezeDance: public Modifier {
public:
  FreezeDance(Tonuino &tonuino, Mp3 &mp3, const Settings &settings): Modifier(tonuino, mp3, settings) {}
  void   loop     () final;
  mode_t getActive() final { return mode_t::freeze_dance; }
  void   init     () final { setNextStopAtMillis(); }

  void setNextStopAtMillis();

private:
  unsigned long nextStopAtMillis       =  0;
  const uint8_t minSecondsBetweenStops =  5;
  const uint8_t maxSecondsBetweenStops = 30;
};

class Locked: public Modifier {
public:
  Locked(Tonuino &tonuino, Mp3 &mp3, const Settings &settings): Modifier(tonuino, mp3, settings) {}
  bool handlePause         () final { LOG(modifier_log, s_debug, F("= Locked::handlePause() -> LOCKED!"))         ; return true; }
  bool handleNextButton    () final { LOG(modifier_log, s_debug, F("= Locked::handleNextButton() -> LOCKED!"))    ; return true; }
  bool handlePreviousButton() final { LOG(modifier_log, s_debug, F("= Locked::handlePreviousButton() -> LOCKED!")); return true; }
  bool handleVolumeUp      () final { LOG(modifier_log, s_debug, F("= Locked::handleVolumeUp() -> LOCKED!"))      ; return true; }
  bool handleVolumeDown    () final { LOG(modifier_log, s_debug, F("= Locked::handleVolumeDown() -> LOCKED!"))    ; return true; }
  bool handleRFID(const nfcTagObject&)
                              final { LOG(modifier_log, s_debug, F("= Locked::handleRFID() -> LOCKED!"))          ; return true; }

  mode_t getActive() final { return mode_t::locked; }
};

class ToddlerMode: public Modifier {
public:
  ToddlerMode(Tonuino &tonuino, Mp3 &mp3, const Settings &settings): Modifier(tonuino, mp3, settings) {}
  bool handlePause         () final { LOG(modifier_log, s_debug, F("= ToddlerMode::handlePause() -> LOCKED!"))         ; return true; }
  bool handleNextButton    () final { LOG(modifier_log, s_debug, F("= ToddlerMode::handleNextButton() -> LOCKED!"))    ; return true; }
  bool handlePreviousButton() final { LOG(modifier_log, s_debug, F("= ToddlerMode::handlePreviousButton() -> LOCKED!")); return true; }
  bool handleVolumeUp      () final { LOG(modifier_log, s_debug, F("= ToddlerMode::handleVolumeUp() -> LOCKED!"))      ; return true; }
  bool handleVolumeDown    () final { LOG(modifier_log, s_debug, F("= ToddlerMode::handleVolumeDown() -> LOCKED!"))    ; return true; }

  mode_t getActive() final { return mode_t::toddler; }
};

class KindergardenMode: public Modifier {
public:
  KindergardenMode(Tonuino &tonuino, Mp3 &mp3, const Settings &settings): Modifier(tonuino, mp3, settings) {}
  bool handleNext() final;

//bool handlePause         () final { LOG(modifier_log, s_debug, F("= KindergardenMode::handlePause() -> LOCKED!"))         ; return true; }
  bool handleNextButton    () final { LOG(modifier_log, s_debug, F("= KindergardenMode::handleNextButton() -> LOCKED!"))    ; return true; }
  bool handlePreviousButton() final { LOG(modifier_log, s_debug, F("= KindergardenMode::handlePreviousButton() -> LOCKED!")); return true; }

  bool   handleRFID(const nfcTagObject &newCard) final;
  mode_t getActive () final { return mode_t::kindergarden; }
  void   init      () final { cardQueued = false; }

private:
  nfcTagObject nextCard{};
  bool cardQueued = false;
};

class RepeatSingleModifier: public Modifier {
public:
  RepeatSingleModifier(Tonuino &tonuino, Mp3 &mp3, const Settings &settings): Modifier(tonuino, mp3, settings) {}
  bool   handleNext    () final;
  bool   handlePrevious() final;
  mode_t getActive     () final { return mode_t::repeat_single; }
};

// An modifier can also do somethings in addition to the modified action
// by returning false (not handled) at the end
// This simple FeedbackModifier will tell the volume before changing it and
// give some feedback once a RFID card is detected.
//class FeedbackModifier: public Modifier {
//public:
//  FeedbackModifier(Tonuino &tonuino, Mp3 &mp3, const Settings &settings): Modifier(tonuino, mp3, settings) {}
//  bool handleVolumeDown() final;
//  bool handleVolumeUp  () final;
//  bool handleRFID      (const nfcTagObject &newCard) final;
//};

#endif /* SRC_MODIFIER_HPP_ */
