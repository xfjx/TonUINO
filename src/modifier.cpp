#include "modifier.hpp"

#include "mp3.hpp"
#include "tonuino.hpp"
#include "logger.hpp"


void SleepTimer::loop() {
  if (sleepAtMillis != 0 && millis() > sleepAtMillis) {
    LOG(modifier_log, s_info, F("=== SleepTimer::loop() -> SLEEP!"));
    mp3.pause();
    tonuino.setStandbyTimer();
    tonuino.resetActiveModifier();
  }
}

void SleepTimer::start(uint8_t minutes) {
  LOG(modifier_log, s_info, F("=== SleepTimer(), minutes: "), minutes);
  sleepAtMillis = millis() + minutes * 60000;
  //playAdvertisement(302);
}

void FreezeDance::loop() {
  if (nextStopAtMillis != 0 && millis() > nextStopAtMillis) {
    LOG(modifier_log, s_info, F("== FreezeDance::loop() -> FREEZE!"));
    if (mp3.isPlaying()) {
      mp3.playAdvertisement(301);
    }
    setNextStopAtMillis();
  }
}

void FreezeDance::setNextStopAtMillis() {
  const uint16_t seconds = random(minSecondsBetweenStops, maxSecondsBetweenStops + 1);
  LOG(modifier_log, s_info, F("=== FreezeDance::setNextStopAtMillis(), seconds: "), seconds);
  nextStopAtMillis = millis() + seconds * 1000;
}

bool KindergardenMode::handleNext() {
  LOG(modifier_log, s_info, F("== KindergardenMode::handleNext() -> NEXT"));
  if (cardQueued) {
    cardQueued = false;

    tonuino.setCard(nextCard);
    LOG(modifier_log, s_info, F("Folder: "), nextCard.nfcFolderSettings.folder, F(" Mode: "), static_cast<uint8_t>(nextCard.nfcFolderSettings.mode));
    tonuino.playFolder();
    return true;
  }
  return false;
}
bool KindergardenMode::handleRFID(const nfcTagObject &newCard) { // lot of work to do!
  LOG(modifier_log, s_info, F("== KindergardenMode::handleRFID() -> queued!"));
  nextCard = newCard;
  cardQueued = true;
  if (!mp3.isPlaying()) {
    handleNext();
  }
  return true;
}

bool RepeatSingleModifier::handleNext() {
  LOG(modifier_log, s_info, F("== RepeatSingleModifier::handleNext() -> REPEAT CURRENT TRACK"));
  delay(50);
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
//  LOG(modifier_log, s_info, F("== FeedbackModifier::handleVolumeDown()!"));
//  return false;
//}
//bool FeedbackModifier::handleVolumeUp() {
//  if (volume < settings.maxVolume) {
//    playAdvertisement(volume + 1, false);
//  } else {
//    playAdvertisement(volume, false);
//  }
//  LOG(modifier_log, s_info, F("== FeedbackModifier::handleVolumeUp()!"));
//  return false;
//}
//bool FeedbackModifier::handleRFID(const nfcTagObject &/*newCard*/) {
//  LOG(modifier_log, s_info, F("== FeedbackModifier::handleRFID()"));
//  return false;
//}




