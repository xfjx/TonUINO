#include "modifier.hpp"

#include "mp3.hpp"
#include "tonuino.hpp"
#include "logger.hpp"
#include "state_machine.hpp"

namespace {

const __FlashStringHelper* str_SleepTimer          () { return F("SleepTimer")  ; }
const __FlashStringHelper* str_FreezeDance         () { return F("FreezeDance") ; }
const __FlashStringHelper* str_KindergardenMode    () { return F("Kita")        ; }
const __FlashStringHelper* str_RepeatSingleModifier() { return F("RepeatSingle"); }

} // anonymous namespace

void SleepTimer::loop() {
  if (sleepTimer.isActive() && sleepTimer.isExpired()) {
    LOG(modifier_log, s_info, str_SleepTimer(), F(" -> SLEEP!"));
    if (SM_tonuino::is_in_state<Play>())
      SM_tonuino::dispatch(button_e(buttonRaw::pause));
    tonuino.resetActiveModifier();
  }
}

void SleepTimer::start(uint8_t minutes) {
  LOG(modifier_log, s_info, str_SleepTimer(), F(" minutes: "), minutes);
  sleepTimer.start(minutes * 60000);
  //playAdvertisement(advertTracks::t_302_sleep);
}

void FreezeDance::loop() {
  if (nextStopAtMillis != 0 && millis() > nextStopAtMillis) {
    LOG(modifier_log, s_info, str_FreezeDance(), F(" -> FREEZE!"));
    if (mp3.isPlaying()) {
      mp3.playAdvertisement(advertTracks::t_301_freeze_freeze);
    }
    setNextStopAtMillis();
  }
}

void FreezeDance::setNextStopAtMillis() {
  const uint16_t seconds = random(minSecondsBetweenStops, maxSecondsBetweenStops + 1);
  LOG(modifier_log, s_info, str_FreezeDance(), F(" next stop at: "), seconds);
  nextStopAtMillis = millis() + seconds * 1000;
}

bool KindergardenMode::handleNext() {
  if (cardQueued) {
    LOG(modifier_log, s_info, str_KindergardenMode(), F(" -> NEXT"));
    cardQueued = false;

    tonuino.setCard(nextCard);
    LOG(modifier_log, s_debug, F("Folder: "), nextCard.nfcFolderSettings.folder, F(" Mode: "), static_cast<uint8_t>(nextCard.nfcFolderSettings.mode));
    tonuino.playFolder();
    return true;
  }
  return false;
}
bool KindergardenMode::handleRFID(const nfcTagObject &newCard) {
  LOG(modifier_log, s_info, str_KindergardenMode(), F(" -> queued!"));
  nextCard = newCard;
  cardQueued = true;
  if (!mp3.isPlaying()) {
    handleNext();
  }
  return true;
}

bool RepeatSingleModifier::handleNext() {
  LOG(modifier_log, s_debug, str_RepeatSingleModifier(), F(" -> REPEAT"));
  delay(50); // TODO remove delay()
  if (!mp3.isPlaying()) {
    mp3.loop(); // this will call Mp3Notify::OnPlayFinished() but will be blocked by lastTrackFinished
    Mp3Notify::ResetLastTrackFinished();
    tonuino.playCurrentTrack();
  }
  return true;
}

//bool FeedbackModifier::handleVolumeDown() {
//  if (volume > settings.minVolume) {
//    playAdvertisement(volume - 1, false);
//  } else {
//    playAdvertisement(volume, false);
//  }
//  LOG(modifier_log, s_info, F("= FeedbackModifier::handleVolumeDown()!"));
//  return false;
//}
//bool FeedbackModifier::handleVolumeUp() {
//  if (volume < settings.maxVolume) {
//    playAdvertisement(volume + 1, false);
//  } else {
//    playAdvertisement(volume, false);
//  }
//  LOG(modifier_log, s_info, F("= FeedbackModifier::handleVolumeUp()!"));
//  return false;
//}
//bool FeedbackModifier::handleRFID(const nfcTagObject &/*newCard*/) {
//  LOG(modifier_log, s_info, F("= FeedbackModifier::handleRFID()"));
//  return false;
//}




