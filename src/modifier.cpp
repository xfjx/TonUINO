#include "modifier.hpp"

#include "mp3.hpp"
#include "tonuino.hpp"


void SleepTimer::loop() {
  if (sleepAtMillis != 0 && millis() > sleepAtMillis) {
    Serial.println(F("=== SleepTimer::loop() -> SLEEP!"));
    mp3.pause();
    tonuino.setStandbyTimer();
    tonuino.resetActiveModifier();
    delete this;
  }
}

void SleepTimer::start(uint8_t minutes) {
  Serial.println(F("=== SleepTimer()"));
  Serial.println(minutes);
  sleepAtMillis = millis() + minutes * 60000;
  //playAdvertisement(302);
}

void FreezeDance::loop() {
  if (nextStopAtMillis != 0 && millis() > nextStopAtMillis) {
    Serial.println(F("== FreezeDance::loop() -> FREEZE!"));
    if (mp3.isPlaying()) {
      mp3.playAdvertisement(301);
    }
    setNextStopAtMillis();
  }
}

void FreezeDance::setNextStopAtMillis() {
  const uint16_t seconds = random(minSecondsBetweenStops, maxSecondsBetweenStops + 1);
  Serial.println(F("=== FreezeDance::setNextStopAtMillis()"));
  Serial.println(seconds);
  nextStopAtMillis = millis() + seconds * 1000;
}

bool KindergardenMode::handleNext() {
  Serial.println(F("== KindergardenMode::handleNext() -> NEXT"));
  if (cardQueued) {
    cardQueued = false;

    tonuino.setCard(nextCard);
    Serial.println(nextCard.nfcFolderSettings.folder);
    //Serial.println(myFolder->mode);
    tonuino.playFolder();
    return true;
  }
  return false;
}
bool KindergardenMode::handleRFID(const nfcTagObject &newCard) { // lot of work to do!
  Serial.println(F("== KindergardenMode::handleRFID() -> queued!"));
  nextCard = newCard;
  cardQueued = true;
  if (!mp3.isPlaying()) {
    handleNext();
  }
  return true;
}

bool RepeatSingleModifier::handleNext() {
  Serial.println(F("== RepeatSingleModifier::handleNext() -> REPEAT CURRENT TRACK"));
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
//  Serial.println(F("== FeedbackModifier::handleVolumeDown()!"));
//  return false;
//}
//bool FeedbackModifier::handleVolumeUp() {
//  if (volume < settings.maxVolume) {
//    playAdvertisement(volume + 1, false);
//  } else {
//    playAdvertisement(volume, false);
//  }
//  Serial.println(F("== FeedbackModifier::handleVolumeUp()!"));
//  return false;
//}
//bool FeedbackModifier::handleRFID(const nfcTagObject &/*newCard*/) {
//  Serial.println(F("== FeedbackModifier::handleRFID()"));
//  return false;
//}




