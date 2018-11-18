/*-----------------------------------------------------------------------------------------
              MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
              Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
  Signal      Pin          Pin           Pin       Pin        Pin              Pin
  -----------------------------------------------------------------------------------------
  RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
  SPI SS      SDA(SS)      10            53        D10        10               10
  SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
  SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
  SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

  button assignments:
  -------------------

  Button B0 (by default pin A0, middle button on the original TonUINO): play+pause
  Button B1 (by default pin A1, right button on the original TonUINO): volume up
  Button B2 (by default pin A2, left button on the original TonUINO): volume down

  additional button actions:
  --------------------------

  Hold B0 for 5 seconds during idle: Enter erase nfc tag mode
  Hold B0 for 5 seconds during playback in story book mode: Reset progress to track 1
  Hold B0 for 2 seconds during erase nfc tag mode: Cancel erase nfc tag mode
  Hold B0 for 2 seconds during nfc tag setup mode: Cancel nfc tag setup mode
  Click B0 during nfc tag setup mode: Confirm selection
  Double click B0 during nfc tag setup mode: Announce folder number or track number

  Hold B1 for 2 seconds during playback in album, party and story book mode: Skip to the next track
  Hold B1 for 2 seconds during nfc tag setup mode: Jump 10 folders or 10 tracks forward

  Hold B2 for 2 seconds during playback in album and story book mode: Skip to the previous track
  Hold B2 for 2 seconds during nfc tag setup mode: Jump 10 folders or 10 tracks backwards

  Hold B0 + B1 + B2 while powering up: Erase the eeprom contents. (Use with care, eeprom write/erase cycles are limited.)

  ir remote:
  ----------

  If a TSOP38238 is connected to pin 5, you can also use an ir remote to remote
  control TonUINO. There are code mappings for the silver apple remote below, but you can
  change them to other codes to match different remotes. This feature can be enabled by
  uncommenting the define TSOP38238 below.

  There is one function, currently only available with the ir remote - box lock.
  When TonUINO is locked, the buttons on TonUINO as well as the nfc reader are disabled
  until TonUINO is unlocked again. Playback continues while TonUINO is locked.

  During playback:
  center - toggle box lock
  play+pause - toggle playback
  up / down - volume up / down
  left / right - previous / next track during album and story book mode, next track during party mode
  menu - reset progress to track 1 in story book mode

  During idle:
  center - toggle box lock
  menu - enter erase nfc tag mode

  During erase nfc tag mode:
  menu - cancel

  During nfc tag setup mode:
  play+pause - confirm
  right - next option
  left - previous option
  up - jump 10 folders or 10 tracks forward
  down - jump 10 folders or 10 tracks backwards
  center - announce folder number or track number
  menu - cancel

  status led:
  -----------

  If a led is connected to pin 6, limited status information is given using that led.
  The led is solid on when TonUINO is playing a track and it is pulsing slowly when
  TonUINO is idle. When TonUINO is in setup new nfc tag or erase nfc tag mode, the
  led is blinking every 500ms. And last but not not least, the led bursts 4 times when
  TonUINO is locked or unlocked. This feature can be enabled by uncommenting the
  define STATUSLED below.

  cubiekid:
  ---------

  If you happen to have a CubieKid case and the additional circuit board, this sketch
  supports both shutdown methods - due to low battery voltage as well as due to
  inactivity after a configurable ammount of time. The shutdown voltage, inactivity time
  as well as other parameters can be setup in the configuration section of this sketch.
  This feature can be enabled by uncommenting the define CUBIEKID below.

  The CubieKid case as well as the additional circuit board, have been designed and
  developed by Jens Hackel and can be found here: https://github.com/jenshackel/CubieKid

  data stored on the nfc tags:
  ----------------------------

  Up to 16 bytes of data are stored in sector 1 / block 4, of which the first
  7 bytes are currently in use.

  13 37 B3 47 01 02 05 00 00 00 00 00 00 00 00 00
  ----------- -- -- --
       |      |  |  |
       |      |  |  +assigned track (0x00-0xFF), only used in single mode
       |      |  +assigned folder (0x01-0x63)
       |      +version, currently always 0x01
       +magic cookie to recognize that a card belongs to TonUINO

  non standard libraries used in this sketch:
  -------------------------------------------

  MFRC522.h - https://github.com/miguelbalboa/rfid
  DFMiniMp3.h - https://github.com/Makuna/DFMiniMp3
  AceButton.h - https://github.com/bxparks/AceButton
  IRremote.h - https://github.com/z3t0/Arduino-IRremote
  Countimer.h - https://github.com/inflop/Countimer
  Vcc.h - https://github.com/Yveaux/Arduino_Vcc
*/

// uncomment the below line to enable ir remote support
// #define TSOP38238

// uncomment the below line to enable status led support
// #define STATUSLED

// uncomment the below line to enable cubiekid support
// #define CUBIEKID

// uncomment the below line to enable additional debug output
// #define DBG

// include required libraries
#include <SoftwareSerial.h>
#include <SPI.h>
#include <EEPROM.h>
#include <MFRC522.h>
#include <DFMiniMp3.h>
#include <AceButton.h>
using namespace ace_button;

// include additional library if ir remote support is enabled
#if defined(TSOP38238)
#include <IRremote.h>
#endif

// include additional libraries if cubiekid support is enabled
#if defined(CUBIEKID)
#include <Countimer.h>
#include <Vcc.h>
#endif

// define global constants
const uint8_t softwareSerialTxPin = 3;              // software serial tx, wired with 1k ohm to rx pin of DFPlayer Mini
const uint8_t softwareSerialRxPin = 2;              // software serial rx, wired straight to tx pin of DFPlayer Mini
const uint8_t mp3BusyPin = 4;                       // reports play state of DFPlayer Mini (LOW = playing)
const uint8_t irReceiverPin = 5;                    // pin used for the ir receiver
const uint8_t statusLedPin = 6;                     // pin used for status led
const uint8_t nfcResetPin = 9;                      // used for spi communication to nfc module
const uint8_t nfcSlaveSelectPin = 10;               // used for spi communication to nfc module
const uint8_t rngSeedPin = A0;                      // used to seed the random number generator
const uint8_t mp3StartVolume = 10;                  // initial volume of DFPlayer Mini
const uint8_t mp3MaxVolume = 25;                    // maximal volume of DFPlayer Mini
const uint8_t button0Pin = A0;                      // middle button
const uint8_t button1Pin = A1;                      // right button
const uint8_t button2Pin = A2;                      // left button
const uint16_t buttonClickDelay = 1000;             // time during which a button click is still a click (in milliseconds)
const uint16_t buttonShortLongPressDelay = 2000;    // time after which a button press is considered a long press (in milliseconds)
const uint16_t buttonLongLongPressDelay = 5000;     // longer long press delay for special cases, i.e. to trigger erase nfc tag mode (in milliseconds)
const uint16_t ledBlinkInterval = 500;              // led blink interval (in milliseconds)
const uint32_t debugConsoleSpeed = 115200;          // speed for the debug console

// define message to mp3 file mappings for spoken feedback
const uint16_t msgSetupNewTag = 305;                // 01
const uint16_t msgSetupNewTagFolderAssigned = 306;  // 02
const uint16_t msgSetupNewTagStoryMode = 310;       // 03
const uint16_t msgSetupNewTagAlbumMode = 320;       // 04
const uint16_t msgSetupNewTagPartyMode = 330;       // 05
const uint16_t msgSetupNewTagSingleMode = 340;      // 06
const uint16_t msgSetupNewTagSingleModeCont = 341;  // 07
const uint16_t msgSetupNewTagStoryBookMode = 350;   // 08
const uint16_t msgSetupNewTagConfirm = 390;         // 09
const uint16_t msgSetupNewTagCancel = 391;          // 10
const uint16_t msgSetupNewTagError = 392;           // 11
const uint16_t msgEraseTag = 405;                   // 12
const uint16_t msgEraseTagConfirm = 490;            // 13
const uint16_t msgEraseTagCancel = 491;             // 14
const uint16_t msgEraseTagError = 492;              // 15
const uint16_t msgWelcome = 505;                    // 16
const uint16_t msgBatteryLow = 510;                 // 17

// used to calculate the total ammount of tracks on the sd card (510 + count from above)
const uint16_t msgCount = 527;

// define code mappings for silver apple tv 2 ir remote
const uint16_t ir1ButtonUp = 0x5057;
const uint16_t ir1ButtonDown = 0x3057;
const uint16_t ir1ButtonLeft = 0x9057;
const uint16_t ir1ButtonRight = 0x6057;
const uint16_t ir1ButtonCenter = 0x3A57;
const uint16_t ir1ButtonMenu = 0xC057;
const uint16_t ir1ButtonPlayPause = 0xFA57;

// define code mappings for silver apple tv 3 ir remote
const uint16_t ir2ButtonUp = 0x50BF;
const uint16_t ir2ButtonDown = 0x30BF;
const uint16_t ir2ButtonLeft = 0x90BF;
const uint16_t ir2ButtonRight = 0x60BF;
const uint16_t ir2ButtonCenter = 0x3ABF;
const uint16_t ir2ButtonMenu = 0xC0BF;
const uint16_t ir2ButtonPlayPause = 0xFABF;

// button actions
enum {NOACTION,
      B0P, B1P, B2P,
      B0H, B1H, B2H,
      B0D, B1D, B2D,
      IRU, IRD, IRL, IRR, IRC, IRM, IRP
     };

// button modes
enum {PAUSE, PLAY, CONFIG};

// this object stores nfc tag data
struct nfcTagObject {
  uint32_t cookie;
  uint8_t version;
  uint8_t assignedFolder;
  uint8_t playbackMode;
  uint8_t assignedTrack;
} nfcTag;

// this object tracks the playback state
struct playbackObject {
  bool firstTrack = true;
  bool queueMode = false;
  uint8_t playTrack = 1;
  uint8_t storedTrack = 1;
  uint16_t folderTrackCount = 0;
  uint16_t lastRandomTrack = 0;
} playback;

#if defined(CUBIEKID)
// this object stores the cubiekid configuration
struct cubiekidObject {
  const uint8_t shutdownPin = 7;                    // pin used to shutdown the system
  const uint8_t powerOffHours = 0;                  // hours until shutdown
  const uint8_t powerOffMinutes = 10;               // minutes until shutdown
  const uint8_t powerOffSeconds = 0;                // seconds until shutdown
  const uint32_t timerInterval = 1000;              // timer interval (in milliseconds)
  const float minVoltage = 4.4;                     // minimum expected voltage level (in volts)
  const float maxVoltage = 5.0;                     // maximum expected voltage level (in volts)
  const float voltageCorrection = 1.0 / 1.0;        // voltage measured by multimeter divided by reported voltage
} cubiekid;
#endif

// global variables
uint8_t inputEvent = NOACTION;

// ############################################################### no configuration below this line ###############################################################

// this function needs to be declared here for the first time because it's called in class Mp3Notify
// the function itself is down below
void playNextTrack(uint16_t globalTrack, bool directionForward, bool triggeredManually);

// used by DFPlayer Mini library during callbacks
class Mp3Notify {
  public:
    static void OnError(uint16_t returnValue) {
      Serial.print(F("mp3 | "));
      switch (returnValue) {
        case DfMp3_Error_Busy:
          Serial.print(F("busy"));
          break;
        case DfMp3_Error_Sleeping:
          Serial.print(F("sleep"));
          break;
        case DfMp3_Error_SerialWrongStack:
          Serial.print(F("serial stack"));
          break;
        case DfMp3_Error_CheckSumNotMatch:
          Serial.print(F("checksum"));
          break;
        case DfMp3_Error_FileIndexOut:
          Serial.print(F("file index"));
          break;
        case DfMp3_Error_FileMismatch:
          Serial.print(F("file mismatch"));
          break;
        case DfMp3_Error_Advertise:
          Serial.print(F("advertise"));
          break;
        case DfMp3_Error_General:
          Serial.print(F("general"));
          break;
        default:
          Serial.print(F("unknown"));
          break;
      }
      Serial.println(F(" error"));
    }
    static void OnPlayFinished(uint16_t returnValue) {
      playNextTrack(returnValue, true, false);
    }
    static void OnCardOnline(uint16_t returnValue) {
      Serial.println(F("mp3 | sd card online "));
    }
    static void OnCardInserted(uint16_t returnValue) {
      Serial.println(F("mp3 | sd card inserted "));
    }
    static void OnCardRemoved(uint16_t returnValue) {
      Serial.println(F("mp3 | sd card removed "));
    }
};

SoftwareSerial secondarySerial(softwareSerialRxPin, softwareSerialTxPin);     // create SoftwareSerial instance
MFRC522 mfrc522(nfcSlaveSelectPin, nfcResetPin);                              // create MFRC522 instance
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(secondarySerial);                    // create DFMiniMp3 instance
ButtonConfig button0Config;                                                   // create ButtonConfig instance
ButtonConfig button1Config;                                                   // create ButtonConfig instance
ButtonConfig button2Config;                                                   // create ButtonConfig instance
AceButton button0(&button0Config);                                            // create AceButton instance
AceButton button1(&button1Config);                                            // create AceButton instance
AceButton button2(&button2Config);                                            // create AceButton instance

#if defined(TSOP38238)
IRrecv irReceiver(irReceiverPin);                                             // create IRrecv instance
decode_results irReadings;                                                    // create decode_results instance to store received ir readings
#endif

#if defined(CUBIEKID)
Countimer cubiekidShutdownTimer;                                              // create Countimer instance
Vcc cubiekidVoltage(cubiekid.voltageCorrection);                              // create Vcc instalce
#endif

// checks all input sources and populates the global inputEvent variable
void checkForInput() {
  // clear inputEvent
  inputEvent = 0;

  // check all buttons
  button0.check();
  button1.check();
  button2.check();

#if defined(TSOP38238)
  // poll ir receiver, has precedence over (overwrites) physical buttons
  if (irReceiver.decode(&irReadings)) {
    switch (irReadings.value & 0xFFFF) {
      // button up
      case ir1ButtonUp:
      case ir2ButtonUp:
        inputEvent = IRU;
        break;
      // button down
      case ir1ButtonDown:
      case ir2ButtonDown:
        inputEvent = IRD;
        break;
      // button left
      case ir1ButtonLeft:
      case ir2ButtonLeft:
        inputEvent = IRL;
        break;
      // button right
      case ir1ButtonRight:
      case ir2ButtonRight:
        inputEvent = IRR;
        break;
      // button center
      case ir1ButtonCenter:
      case ir2ButtonCenter:
        inputEvent = IRC;
        break;
      // button menu
      case ir1ButtonMenu:
      case ir2ButtonMenu:
        inputEvent = IRM;
        break;
      // button play+pause
      case ir1ButtonPlayPause:
      case ir2ButtonPlayPause:
        inputEvent = IRP;
        break;
      default:
        break;
    }
    irReceiver.resume();
  }
#endif
}

// translates the various button events into enums
void translateButtonInput(AceButton* button, uint8_t eventType, uint8_t /* buttonState */) {
  switch (button->getId()) {
    // botton 0 (middle)
    case 0:
      switch (eventType) {
        case AceButton::kEventClicked:
          inputEvent = B0P;
          break;
        case AceButton::kEventLongPressed:
          inputEvent = B0H;
          break;
        case AceButton::kEventDoubleClicked:
          inputEvent = B0D;
          break;
        default:
          break;
      }
      break;
    // botton 1 (right)
    case 1:
      switch (eventType) {
        case AceButton::kEventClicked:
          inputEvent = B1P;
          break;
        case AceButton::kEventLongPressed:
          inputEvent = B1H;
          break;
        default:
          break;
      }
      break;
    // botton 2 (left)
    case 2:
      switch (eventType) {
        case AceButton::kEventClicked:
          inputEvent = B2P;
          break;
        case AceButton::kEventLongPressed:
          inputEvent = B2H;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

// switches button configuration dependig on the state that TonUINO is in
void switchButtonConfiguration(uint8_t buttonMode) {
  // default configuration for all modes
  // button 0
  button0Config.setFeature(ButtonConfig::kFeatureClick);
  button0Config.setFeature(ButtonConfig::kFeatureLongPress);
  button0Config.setClickDelay(buttonClickDelay);
  // button 1
  button1Config.setFeature(ButtonConfig::kFeatureClick);
  button1Config.setFeature(ButtonConfig::kFeatureLongPress);
  button1Config.setClickDelay(buttonClickDelay);
  button1Config.setLongPressDelay(buttonShortLongPressDelay);
  // button 2
  button2Config.setFeature(ButtonConfig::kFeatureClick);
  button2Config.setFeature(ButtonConfig::kFeatureLongPress);
  button2Config.setClickDelay(buttonClickDelay);
  button2Config.setLongPressDelay(buttonShortLongPressDelay);

  // non default configuration
  switch (buttonMode) {
    case PAUSE:
      // button 0
      button0Config.setLongPressDelay(buttonLongLongPressDelay);
      button0Config.clearFeature(ButtonConfig::kFeatureDoubleClick);
      button0Config.clearFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
      break;
    case PLAY:
      // button 0
      button0Config.setLongPressDelay(buttonLongLongPressDelay);
      button0Config.clearFeature(ButtonConfig::kFeatureDoubleClick);
      button0Config.clearFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
      break;
    case CONFIG:
      // button 0
      button0Config.setLongPressDelay(buttonShortLongPressDelay);
      button0Config.setFeature(ButtonConfig::kFeatureDoubleClick);
      button0Config.setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
      break;
    default:
      break;
  }
}

// waits for current playing track to finish
void waitPlaybackToFinish() {
  delay(500);
  do {
    delay(10);
  } while (!digitalRead(mp3BusyPin));
}

// plays next track depending on the current playback mode
void playNextTrack(uint16_t globalTrack, bool directionForward, bool triggeredManually) {
  static uint16_t lastCallTrack = 0;

  //delay 100ms to be on the safe side with the serial communication
  delay(100);

  // we only advance to a new track when in queue mode, not during interactive prompt playback (ie. during configuration of a new nfc tag)
  if (!playback.queueMode) return;

  // story mode: play one random track in folder
  // there is no next track in story mode > stop playback
  if (nfcTag.playbackMode == 1) {
    playback.queueMode = false;
    switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
    cubiekidShutdownTimer.restart();
#endif
    Serial.println(F("mp3 | story mode > stop"));
    mp3.stop();
  }

  // album mode: play complete folder
  // advance to the next track, stop if the end of the folder is reached or go back to the previous track
  if (nfcTag.playbackMode == 2) {

    // **workaround for some DFPlayer mini modules that make two callbacks in a row when finishing a track**
    // reset lastCallTrack to avoid lockup when playback was just started
    if (playback.firstTrack) {
      playback.firstTrack = false;
      lastCallTrack = 0;
    }
    // check if we get called with the same track number twice in a row, if yes return immediately
    if (lastCallTrack == globalTrack) return;
    else lastCallTrack = globalTrack;

    // play next track?
    if (directionForward) {
      // there are more tracks after the current one, play next track
      if (playback.playTrack < playback.folderTrackCount) {
        playback.playTrack++;
        Serial.print(F("mp3 | album mode > folder "));
        Serial.print(nfcTag.assignedFolder);
        Serial.print(F(" > track "));
        Serial.print(playback.playTrack);
        Serial.print(F("/"));
        Serial.println(playback.folderTrackCount);
        mp3.playFolderTrack(nfcTag.assignedFolder, playback.playTrack);
      }
      // there are no more tracks after the current one
      else {
        // user wants to manually play the next track, ignore the next track command
        if (triggeredManually) Serial.println(F("mp3 | album mode > end of folder"));
        // stop playback
        else {
          playback.queueMode = false;
          switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
          cubiekidShutdownTimer.restart();
#endif
          Serial.println(F("mp3 | album mode > end of folder > stop"));
          mp3.stop();
        }
      }
    }
    // play previous track?
    else {
      // there are more tracks before the current one, play the previous track
      if (playback.playTrack > 1) {
        playback.playTrack--;
        Serial.print(F("mp3 | album mode > folder "));
        Serial.print(nfcTag.assignedFolder);
        Serial.print(F(" > track "));
        Serial.print(playback.playTrack);
        Serial.print(F("/"));
        Serial.println(playback.folderTrackCount);
        mp3.playFolderTrack(nfcTag.assignedFolder, playback.playTrack);
      }
      // there are no more tracks before the current one, ignore the previous track command
      else Serial.println(F("mp3 | album mode > beginning of folder"));
    }
  }

  // party mode: shuffle tracks in folder indefinitely
  // just play another random track
  if (nfcTag.playbackMode == 3) {
    playback.playTrack = random(1, playback.folderTrackCount + 1);
    if (playback.playTrack == playback.lastRandomTrack) playback.playTrack = playback.playTrack == playback.folderTrackCount ? 1 : playback.playTrack + 1;
    playback.lastRandomTrack = playback.playTrack;
    Serial.print(F("mp3 | party mode > folder "));
    Serial.print(nfcTag.assignedFolder);
    Serial.print(F(" > track "));
    Serial.print(playback.playTrack);
    Serial.print(F("/"));
    Serial.println(playback.folderTrackCount);
    mp3.playFolderTrack(nfcTag.assignedFolder, playback.playTrack);
  }

  // single mode: play a single track in folder
  // there is no next track in single mode > stop playback
  if (nfcTag.playbackMode == 4) {
    playback.queueMode = false;
    switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
    cubiekidShutdownTimer.restart();
#endif
    Serial.println(F("mp3 | single mode > stop"));
    mp3.stop();
  }

  // story book mode: play complete folder and track progress
  // advance to the next track, stop if the end of the folder is reached or go back to the previous track
  if (nfcTag.playbackMode == 5) {

    // **workaround for some DFPlayer mini modules that make two callbacks in a row when finishing a track**
    // reset lastCallTrack to avoid lockup when playback was just started
    if (playback.firstTrack) {
      playback.firstTrack = false;
      lastCallTrack = 0;
    }
    // check if we get called with the same track number twice in a row, if yes return immediately
    if (lastCallTrack == globalTrack) return;
    else lastCallTrack = globalTrack;

    // play next track?
    if (directionForward) {
      // there are more tracks after the current one, play next track
      if (playback.playTrack < playback.folderTrackCount) {
        playback.playTrack++;
        Serial.print(F("mp3 | story book mode > folder "));
        Serial.print(nfcTag.assignedFolder);
        Serial.print(F(" > track "));
        Serial.print(playback.playTrack);
        Serial.print(F("/"));
        Serial.println(playback.folderTrackCount);
        mp3.playFolderTrack(nfcTag.assignedFolder, playback.playTrack);
      }
      // there are no more tracks after the current one
      else {
        // user wants to manually play the next track, ignore the next track command
        if (triggeredManually) Serial.println(F("mp3 | story book mode > end of folder"));
        // stop playback and reset progress
        else {
          playback.queueMode = false;
          EEPROM.update(nfcTag.assignedFolder, 0);
          switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
          cubiekidShutdownTimer.restart();
#endif
          Serial.println(F("mp3 | story book mode > end of folder > stop > progress reset"));
          mp3.stop();
        }
      }
    }
    // play previous track?
    else {
      // there are more tracks before the current one, play the previous track
      if (playback.playTrack > 1) {
        playback.playTrack--;
        Serial.print(F("mp3 | story book mode > folder "));
        Serial.print(nfcTag.assignedFolder);
        Serial.print(F(" > track "));
        Serial.print(playback.playTrack);
        Serial.print(F("/"));
        Serial.println(playback.folderTrackCount);
        mp3.playFolderTrack(nfcTag.assignedFolder, playback.playTrack);
      }
      // there are no more tracks before the current one, ignore the previous track command
      else Serial.println(F("mp3 | story book mode > beginning of folder"));
    }
  }
}

// reads data from nfc tag
uint8_t readNfcTagData() {
  uint8_t returnCode;
  uint8_t mifareBlock = 4;
  uint8_t mifareData[18];
  uint8_t mifareDataSize = sizeof(mifareData);
  MFRC522::StatusCode mifareStatus;
  MFRC522::MIFARE_Key mifareKey;
  MFRC522::PICC_Type mifareType;
  for (uint8_t i = 0; i < 6; i++) mifareKey.keyByte[i] = 0xFF;

  // check if card type is supported
  mifareType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (mifareType != MFRC522::PICC_TYPE_MIFARE_MINI && mifareType != MFRC522::PICC_TYPE_MIFARE_1K && mifareType != MFRC522::PICC_TYPE_MIFARE_4K) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    returnCode = 255;
  }
  else {
    // check if we can authenticate with mifareKey
    mifareStatus = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, mifareBlock, &mifareKey, &(mfrc522.uid));
    if (mifareStatus != MFRC522::STATUS_OK) {
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      returnCode = 254;
    }
    else {
      // read data from nfc tag
      mifareStatus = (MFRC522::StatusCode)mfrc522.MIFARE_Read(mifareBlock, mifareData, &mifareDataSize);
      if (mifareStatus != MFRC522::STATUS_OK) {
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        returnCode = 253;
      }
      else {
#if defined(DBG)
        // for debug purposes, print the first 16 bytes of sector 1 / block 4
        Serial.print(F("dbg |"));
        for (uint8_t i = 0; i < 16; i++) {
          Serial.print(mifareData[i] < 0x10 ? " 0" : " ");
          Serial.print(mifareData[i], HEX);
        }
        Serial.println();
#endif

        // convert 4 byte cookie to 32bit decimal for easier handling
        uint32_t tempCookie;
        tempCookie  = (uint32_t)mifareData[0] << 24;
        tempCookie += (uint32_t)mifareData[1] << 16;
        tempCookie += (uint32_t)mifareData[2] << 8;
        tempCookie += (uint32_t)mifareData[3];

        // if cookie is not blank, update ncfTag object with data read from nfc tag
        if (tempCookie != 0) {
          mfrc522.PICC_HaltA();
          mfrc522.PCD_StopCrypto1();
          nfcTag.cookie = tempCookie;
          nfcTag.version = mifareData[4];
          nfcTag.assignedFolder = mifareData[5];
          nfcTag.playbackMode = mifareData[6];
          nfcTag.assignedTrack = mifareData[7];
        }
        // if cookie is blank, clear ncfTag object
        else {
          nfcTag.cookie = 0;
          nfcTag.version = 0;
          nfcTag.assignedFolder = 0;
          nfcTag.playbackMode = 0;
          nfcTag.assignedTrack = 0;
        }
        returnCode = 1;
      }
    }
  }

  return returnCode;
}

// writes data to nfc tag
uint8_t writeNfcTagData(uint8_t mifareData[], uint8_t mifareDataSize) {
  uint8_t returnCode;
  uint8_t mifareBlock = 4;
  uint8_t mifareTrailerBlock = 7;
  MFRC522::StatusCode mifareStatus;
  MFRC522::MIFARE_Key mifareKey;
  MFRC522::PICC_Type mifareType;
  for (uint8_t i = 0; i < 6; i++) mifareKey.keyByte[i] = 0xFF;

  // check if card type is supported
  mifareType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (mifareType != MFRC522::PICC_TYPE_MIFARE_MINI && mifareType != MFRC522::PICC_TYPE_MIFARE_1K && mifareType != MFRC522::PICC_TYPE_MIFARE_4K) returnCode = 255;
  else {
    // check if we can authenticate with mifareKey
    mifareStatus = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, mifareTrailerBlock, &mifareKey, &(mfrc522.uid));
    if (mifareStatus != MFRC522::STATUS_OK) returnCode = 254;
    else {
      // write data to nfc tag
      mifareStatus = (MFRC522::StatusCode)mfrc522.MIFARE_Write(mifareBlock, mifareData, mifareDataSize);
      if (mifareStatus != MFRC522::STATUS_OK) returnCode = 253;
      else returnCode = 1;
    }
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  return returnCode;
}

#if defined(STATUSLED)
// fade in/out status led while beeing idle, during playback set to full brightness
void fadeStatusLed(bool isPlaying) {
  static bool statusLedDirection = false;
  static int16_t statusLedValue = 255;
  static uint32_t statusLedOldMillis;

  // TonUINO is playing, set status led to full brightness
  if (isPlaying) {
    statusLedValue = 255;
    digitalWrite(statusLedPin, true);
  }
  // TonUINO is not playing, fade status led in/out
  else {
    uint32_t statusLedNewMillis = millis();
    if (statusLedNewMillis - statusLedOldMillis >= 100) {
      statusLedOldMillis = statusLedNewMillis;
      if (statusLedDirection) {
        statusLedValue += 10;
        if (statusLedValue >= 255) {
          statusLedValue = 255;
          statusLedDirection = !statusLedDirection;
        }
      }
      else {
        statusLedValue -= 10;
        if (statusLedValue <= 0) {
          statusLedValue = 0;
          statusLedDirection = !statusLedDirection;
        }
      }
      analogWrite(statusLedPin, statusLedValue);
    }
  }
}

// blink status led every blinkInterval milliseconds
void blinkStatusLed(uint16_t blinkInterval) {
  static bool statusLedState;
  static uint32_t statusLedOldMillis;

  uint32_t statusLedNewMillis = millis();
  if (statusLedNewMillis - statusLedOldMillis >= blinkInterval) {
    statusLedOldMillis = statusLedNewMillis;
    statusLedState = !statusLedState;
    digitalWrite(statusLedPin, statusLedState);
  }
}

// burst status led 4 times
void burstStatusLed() {
  bool statusLedState = true;

  for (uint8_t i = 0; i < 8; i++) {
    statusLedState = !statusLedState;
    digitalWrite(statusLedPin, statusLedState);
    delay(100);
  }
}
#endif

#if defined(CUBIEKID)
// show time remaining until shutdown
// available when DEBUG is enabled
void cubiekidTimerRemaining() {
#if defined(DBG)
  Serial.print(F("dbg | shutdown in "));
  Serial.println(cubiekidShutdownTimer.getCurrentTime());
#endif
}

// shutdown the system
void cubiekidShutdown() {
  Serial.println(F("sys | inactivity, shutdown..."));
  digitalWrite(cubiekid.shutdownPin, LOW);
}
#endif

void setup() {
#if defined(CUBIEKID)
  pinMode(cubiekid.shutdownPin, OUTPUT);
  digitalWrite(cubiekid.shutdownPin, HIGH);
#endif
  Serial.begin(debugConsoleSpeed);
  while (!Serial);
  Serial.println(F("sys | TonUINO JUKEBOX"));
  Serial.println(F("sys | by Thorsten Voß"));
  Serial.println(F("sys | Stephan Eisfeld"));
  Serial.println(F("---------------------"));
  Serial.println(F("sys | initializing nfc module"));
  SPI.begin();
  mfrc522.PCD_Init();
#if defined(DBG)
  mfrc522.PCD_DumpVersionToSerial();
#endif

  Serial.println(F("sys | initializing mp3 module"));
  mp3.begin();
  delay(2000);
  Serial.print(F("sys |    volume: "));
  Serial.println(mp3StartVolume);
  mp3.setVolume(mp3StartVolume);
  Serial.print(F("sys |    tracks: "));
  Serial.println(mp3.getTotalTrackCount() - msgCount);
  pinMode(mp3BusyPin, INPUT);

  Serial.println(F("sys | initializing rng"));
  randomSeed(analogRead(rngSeedPin));

  Serial.println(F("sys | initializing buttons"));
  pinMode(button0Pin, INPUT_PULLUP);
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  button0.init(button0Pin, HIGH, 0);
  button1.init(button1Pin, HIGH, 1);
  button2.init(button2Pin, HIGH, 2);
  button0Config.setEventHandler(translateButtonInput);
  button1Config.setEventHandler(translateButtonInput);
  button2Config.setEventHandler(translateButtonInput);
  switchButtonConfiguration(PAUSE);

#if defined(TSOP38238)
  Serial.println(F("sys | initializing ir receiver"));
  irReceiver.enableIRIn();
#endif

#if defined(STATUSLED)
  Serial.println(F("sys | initializing status led"));
  pinMode(statusLedPin, OUTPUT);
  digitalWrite(statusLedPin, HIGH);
#endif

#if defined(CUBIEKID)
  Serial.println(F("sys | initializing cubiekid"));
  Serial.print(F("sys |  expected: "));
  Serial.print(cubiekid.maxVoltage);
  Serial.println(F("V"));
  Serial.print(F("sys |  shutdown: "));
  Serial.print(cubiekid.minVoltage);
  Serial.println(F("V"));
  Serial.print(F("sys |   current: "));
  Serial.print(cubiekidVoltage.Read_Volts());
  Serial.print(F("V ("));
  Serial.print(cubiekidVoltage.Read_Perc(cubiekid.minVoltage, cubiekid.maxVoltage));
  Serial.println(F("%)"));
  Serial.print(F("sys |     timer: "));
  Serial.print(cubiekid.powerOffHours);
  Serial.print(F("h:"));
  Serial.print(cubiekid.powerOffMinutes);
  Serial.print(F("m:"));
  Serial.print(cubiekid.powerOffSeconds);
  Serial.println(F("s"));
  cubiekidShutdownTimer.setCounter(cubiekid.powerOffHours, cubiekid.powerOffMinutes, cubiekid.powerOffSeconds, cubiekidShutdownTimer.COUNT_DOWN, cubiekidShutdown);
  cubiekidShutdownTimer.setInterval(cubiekidTimerRemaining, cubiekid.timerInterval);
  cubiekidShutdownTimer.start();
#endif

  // hold down all three buttons while powering up: erase the eeprom contents
  if (digitalRead(button0Pin) == LOW && digitalRead(button1Pin) == LOW && digitalRead(button2Pin) == LOW) {
    Serial.println(F("sys | initializing eeprom"));
    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.update(i, 0);
#if defined(STATUSLED)
      blinkStatusLed(50);
#endif
    }
  }

  Serial.println(F("sys | system is ready"));
  mp3.playMp3FolderTrack(msgWelcome);
}

void loop() {
  static bool isLocked = false;
  bool isPlaying = !digitalRead(mp3BusyPin);
  checkForInput();

#if defined(STATUSLED)
  fadeStatusLed(isPlaying);
#endif

#if defined(CUBIEKID)
  // update timer
  cubiekidShutdownTimer.run();
  // if low voltage level is reached, store progress and shutdown
  if (cubiekidVoltage.Read_Volts() <= cubiekid.minVoltage) {
    // if the current playback mode is story book mode: store the current progress
    if (nfcTag.playbackMode == 5) EEPROM.update(nfcTag.assignedFolder, playback.playTrack);
    mp3.playMp3FolderTrack(msgBatteryLow);
    delay(10000);
    Serial.println(F("sys | low voltage, shutdown..."));
    digitalWrite(cubiekid.shutdownPin, LOW);
  }
#endif

  // ################################################################################
  // # main code block, if nfc tag is detected and TonUINO is not locked do something
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() && !isLocked) {
    // if the current playback mode is story book mode, only while playing: store the current progress
    if (nfcTag.playbackMode == 5 && isPlaying) {
      Serial.print(F("mp3 | story book mode > folder "));
      Serial.print(nfcTag.assignedFolder);
      Serial.print(F(" > track "));
      Serial.print(playback.playTrack);
      Serial.print(F("/"));
      Serial.print(playback.folderTrackCount);
      Serial.println(F(" > progress saved"));
      EEPROM.update(nfcTag.assignedFolder, playback.playTrack);
    }
    Serial.println(F("nfc | tag detected"));
    uint8_t readNfcTagStatus = readNfcTagData();
    // ##############################
    // # nfc tag is successfully read
    if (readNfcTagStatus == 1) {
      Serial.println(F("nfc | successfully read tag"));
      // #######################################################################################################
      // # nfc tag has our magic cookie 0x1337 0xb347 on it (322417479), use data from nfc tag to start playback
      if (nfcTag.cookie == 322417479) {
        switchButtonConfiguration(PLAY);
#if defined(CUBIEKID)
        cubiekidShutdownTimer.stop();
#endif
        // print read data to console
        Serial.println(F("nfc | tag is one of ours"));
        Serial.print(F("nfc |   folder: "));
        Serial.println(nfcTag.assignedFolder);
        Serial.print(F("nfc |     mode: "));
        switch (nfcTag.playbackMode) {
          case 1:
            Serial.println(F("story mode"));
            break;
          case 2:
            Serial.println(F("album mode"));
            break;
          case 3:
            Serial.println(F("party mode"));
            break;
          case 4:
            Serial.println(F("single mode"));
            Serial.print(F("nfc |    track: "));
            Serial.println(nfcTag.assignedTrack);
            break;
          case 5:
            Serial.println(F("story book mode"));
            break;
          default:
            break;
        }
        // start playback
        Serial.println(F("sys | starting playback"));
        playback.folderTrackCount = mp3.getFolderTrackCount(nfcTag.assignedFolder);
        switch (nfcTag.playbackMode) {
          // story mode
          case 1:
            playback.playTrack = random(1, playback.folderTrackCount + 1);
            Serial.print(F("mp3 | story mode > randomly play one of "));
            Serial.print(playback.folderTrackCount);
            Serial.print(F(" tracks in folder "));
            Serial.println(nfcTag.assignedFolder);
            Serial.print(F("mp3 | story mode > folder "));
            Serial.print(nfcTag.assignedFolder);
            Serial.print(F(" > track "));
            Serial.print(playback.playTrack);
            Serial.print(F("/"));
            Serial.println(playback.folderTrackCount);
            break;
          // album mode
          case 2:
            playback.playTrack = 1;
            Serial.print(F("mp3 | album mode > sequentially play all "));
            Serial.print(playback.folderTrackCount);
            Serial.print(F(" tracks in folder "));
            Serial.println(nfcTag.assignedFolder);
            Serial.print(F("mp3 | album mode > folder "));
            Serial.print(nfcTag.assignedFolder);
            Serial.print(F(" > track "));
            Serial.print(playback.playTrack);
            Serial.print(F("/"));
            Serial.println(playback.folderTrackCount);
            break;
          // party mode
          case 3:
            playback.playTrack = random(1, playback.folderTrackCount + 1);
            playback.lastRandomTrack = playback.playTrack;
            Serial.print(F("mp3 | party mode > shuffle all "));
            Serial.print(playback.folderTrackCount);
            Serial.print(F(" tracks in folder "));
            Serial.println(nfcTag.assignedFolder);
            Serial.print(F("mp3 | party mode > folder "));
            Serial.print(nfcTag.assignedFolder);
            Serial.print(F(" > track "));
            Serial.print(playback.playTrack);
            Serial.print(F("/"));
            Serial.println(playback.folderTrackCount);
            break;
          // single mode
          case 4:
            playback.playTrack = nfcTag.assignedTrack;
            Serial.println(F("mp3 | single mode > play single track"));
            Serial.print(F("mp3 | single mode > folder "));
            Serial.print(nfcTag.assignedFolder);
            Serial.print(F(" > track "));
            Serial.println(playback.playTrack);
            break;
          // story book mode
          case 5:
            playback.playTrack = 1;
            Serial.print(F("mp3 | story book mode > sequentially play all "));
            Serial.print(playback.folderTrackCount);
            Serial.print(F(" tracks in folder "));
            Serial.print(nfcTag.assignedFolder);
            Serial.println(F(" and track progress"));
            playback.storedTrack = EEPROM.read(nfcTag.assignedFolder);
            // don't resume from eeprom, play from the beginning
            if (playback.storedTrack == 0 || playback.storedTrack > playback.folderTrackCount) playback.playTrack = 1;
            // resume from eeprom
            else {
              playback.playTrack = playback.storedTrack;
              Serial.print(F("mp3 | story book mode > resuming with track "));
              Serial.println(playback.storedTrack);
            }
            Serial.print(F("mp3 | story book mode > folder "));
            Serial.print(nfcTag.assignedFolder);
            Serial.print(F(" > track "));
            Serial.print(playback.playTrack);
            Serial.print(F("/"));
            Serial.println(playback.folderTrackCount);
            break;
          default:
            break;
        }
        playback.firstTrack = true;
        playback.queueMode = true;
        mp3.playFolderTrack(nfcTag.assignedFolder, playback.playTrack);
      }
      // # end - nfc tag has our magic cookie 0x1337 0xb347 on it (322417479)
      // ####################################################################

      // #######################################################################################################
      // # nfc tag does not have our magic cookie 0x1337 0xb347 on it (0), start setup to configure this nfc tag
      else if (nfcTag.cookie == 0) {
        switchButtonConfiguration(CONFIG);
#if defined(CUBIEKID)
        cubiekidShutdownTimer.stop();
#endif
        Serial.println(F("nfc | tag is blank"));
        Serial.println(F("sys | starting tag setup"));
        // let user select the folder to assign
        playback.queueMode = false;
        bool setAssignedFolder = false;
        Serial.println(F("sys |   select folder"));
        mp3.playMp3FolderTrack(msgSetupNewTag);
        // loop until folder is assigned
        do {
          checkForInput();
          // button 0 (middle) press or ir remote play+pause: confirm selected folder
          if (inputEvent == B0P || inputEvent == IRP) {
            if (nfcTag.assignedFolder == 0) {
              Serial.println(F("sys |     no folder selected"));
              continue;
            }
            else setAssignedFolder = true;
          }
          // button 0 (middle) double click or ir remote center: announce folder number
          else if (inputEvent == B0D || inputEvent == IRC) {
            mp3.playAdvertisement(nfcTag.assignedFolder);
          }
          // button 1 (right) press or ir remote right: next folder
          else if (inputEvent == B1P || inputEvent == IRR) {
            nfcTag.assignedFolder = min(nfcTag.assignedFolder + 1, 99);
            Serial.print(F("sys |     folder "));
            Serial.println(nfcTag.assignedFolder);
            mp3.playFolderTrack(nfcTag.assignedFolder, 1);
          }
          // button 2 (left) press or ir remote left: previous folder
          else if (inputEvent == B2P || inputEvent == IRL) {
            nfcTag.assignedFolder = max(nfcTag.assignedFolder - 1, 1);
            Serial.print(F("sys |     folder "));
            Serial.println(nfcTag.assignedFolder);
            mp3.playFolderTrack(nfcTag.assignedFolder, 1);
          }
          // button 0 (middle) hold for 2 sec or ir remote menu: cancel tag setup
          else if (inputEvent == B0H || inputEvent == IRM) {
            switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
            cubiekidShutdownTimer.start();
#endif
            Serial.println(F("sys | tag setup canceled"));
            nfcTag.assignedFolder = 0;
            nfcTag.playbackMode = 0;
            nfcTag.assignedTrack = 0;
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            mp3.playMp3FolderTrack(msgSetupNewTagCancel);
            return;
          }
          // button 1 (right) hold or ir remote up: jump 10 folders forward
          else if (inputEvent == B1H || inputEvent == IRU) {
            nfcTag.assignedFolder = min(nfcTag.assignedFolder + 10, 99);
            Serial.print(F("sys |     folder "));
            Serial.println(nfcTag.assignedFolder);
            mp3.playMp3FolderTrack(nfcTag.assignedFolder);
            waitPlaybackToFinish();
            mp3.playFolderTrack(nfcTag.assignedFolder, 1);
          }
          // button 2 (left) hold or ir remote down: jump 10 folders backwards
          else if (inputEvent == B2H || inputEvent == IRD) {
            nfcTag.assignedFolder = max(nfcTag.assignedFolder - 10, 1);
            Serial.print(F("sys |     folder "));
            Serial.println(nfcTag.assignedFolder);
            mp3.playMp3FolderTrack(nfcTag.assignedFolder);
            waitPlaybackToFinish();
            mp3.playFolderTrack(nfcTag.assignedFolder, 1);
          }
#if defined(STATUSLED)
          blinkStatusLed(ledBlinkInterval);
#endif
          mp3.loop();
        }
        while (!setAssignedFolder);
        delay(500);
        Serial.print(F("sys |     folder "));
        Serial.print(nfcTag.assignedFolder);
        Serial.println(F(" selected"));
        // let user select playback mode
        bool setPlaybackMode = false;
        Serial.println(F("sys |   select playback mode"));
        mp3.playMp3FolderTrack(msgSetupNewTagFolderAssigned);
        // loop until playback mode is set
        do {
          checkForInput();
          // button 0 (middle) press or ir remote play+pause: confirm selected playback mode
          if (inputEvent == B0P || inputEvent == IRP) {
            if (nfcTag.playbackMode == 0) {
              Serial.println(F("sys |     no playback mode selected"));
              continue;
            }
            else setPlaybackMode = true;
          }
          // button 1 (right) press or ir remote up / right: next playback mode
          else if (inputEvent == B1P || inputEvent == IRU || inputEvent == IRR) {
            nfcTag.playbackMode = min(nfcTag.playbackMode + 1, 5);
            Serial.print(F("sys |     "));
            switch (nfcTag.playbackMode) {
              case 1:
                Serial.println(F("story mode"));
                mp3.playMp3FolderTrack(msgSetupNewTagStoryMode);
                break;
              case 2:
                Serial.println(F("album mode"));
                mp3.playMp3FolderTrack(msgSetupNewTagAlbumMode);
                break;
              case 3:
                Serial.println(F("party mode"));
                mp3.playMp3FolderTrack(msgSetupNewTagPartyMode);
                break;
              case 4:
                Serial.println(F("single mode"));
                mp3.playMp3FolderTrack(msgSetupNewTagSingleMode);
                break;
              case 5:
                Serial.println(F("story book mode"));
                mp3.playMp3FolderTrack(msgSetupNewTagStoryBookMode);
                break;
              default:
                break;
            }
          }
          // button 2 (left) press or ir remote down / left: previous playback mode
          else if (inputEvent == B2P || inputEvent == IRD || inputEvent == IRL) {
            nfcTag.playbackMode = max(nfcTag.playbackMode - 1, 1);
            Serial.print(F("sys |     "));
            switch (nfcTag.playbackMode) {
              case 1:
                Serial.println(F("story mode"));
                mp3.playMp3FolderTrack(msgSetupNewTagStoryMode);
                break;
              case 2:
                Serial.println(F("album mode"));
                mp3.playMp3FolderTrack(msgSetupNewTagAlbumMode);
                break;
              case 3:
                Serial.println(F("party mode"));
                mp3.playMp3FolderTrack(msgSetupNewTagPartyMode);
                break;
              case 4:
                Serial.println(F("single mode"));
                mp3.playMp3FolderTrack(msgSetupNewTagSingleMode);
                break;
              case 5:
                Serial.println(F("story book mode"));
                mp3.playMp3FolderTrack(msgSetupNewTagStoryBookMode);
                break;
              default:
                break;
            }
          }
          // button 0 (middle) hold for 2 sec or ir remote menu: cancel tag setup
          else if (inputEvent == B0H || inputEvent == IRM) {
            switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
            cubiekidShutdownTimer.start();
#endif
            Serial.println(F("sys | tag setup canceled"));
            nfcTag.assignedFolder = 0;
            nfcTag.playbackMode = 0;
            nfcTag.assignedTrack = 0;
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            mp3.playMp3FolderTrack(msgSetupNewTagCancel);
            return;
          }
#if defined(STATUSLED)
          blinkStatusLed(ledBlinkInterval);
#endif
          mp3.loop();
        }
        while (!setPlaybackMode);
        delay(500);
        Serial.print(F("sys |     "));
        switch (nfcTag.playbackMode) {
          case 1:
            Serial.print(F("story mode"));
            break;
          case 2:
            Serial.print(F("album mode"));
            break;
          case 3:
            Serial.print(F("party mode"));
            break;
          case 4:
            Serial.print(F("single mode"));
            break;
          case 5:
            Serial.print(F("story book mode"));
            break;
          default:
            break;
        }
        Serial.println(F(" selected"));
        // if single mode was selected, let user select the track to assign
        if (nfcTag.playbackMode == 4) {
          bool setAssignedTrack = false;
          Serial.println(F("sys |   select track"));
          mp3.playMp3FolderTrack(msgSetupNewTagSingleModeCont);
          // loop until track is assigned
          do {
            checkForInput();
            // button 0 (middle) press or ir remote play+pause: confirm selected track
            if (inputEvent == B0P || inputEvent == IRP) {
              if (nfcTag.assignedTrack == 0) {
                Serial.println(F("sys |     no track selected"));
                continue;
              }
              else setAssignedTrack = true;
            }
            // button 0 (middle) double click or ir remote center: announce track number
            else if (inputEvent == B0D || inputEvent == IRC) {
              mp3.playAdvertisement(nfcTag.assignedTrack);
            }
            // button 1 (right) press or ir remote right: next track
            else if (inputEvent == B1P || inputEvent == IRR) {
              nfcTag.assignedTrack = min(nfcTag.assignedTrack + 1, 255);
              Serial.print(F("sys |     track "));
              Serial.println(nfcTag.assignedTrack);
              mp3.playFolderTrack(nfcTag.assignedFolder, nfcTag.assignedTrack);
            }
            // button 2 (left) press or ir remote left: previous track
            else if (inputEvent == B2P || inputEvent == IRL) {
              nfcTag.assignedTrack = max(nfcTag.assignedTrack - 1, 1);
              Serial.print(F("sys |     track "));
              Serial.println(nfcTag.assignedTrack);
              mp3.playFolderTrack(nfcTag.assignedFolder, nfcTag.assignedTrack);
            }
            // button 0 (middle) hold for 2 sec or ir remote menu: cancel tag setup
            else if (inputEvent == B0H || inputEvent == IRM) {
              switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
              cubiekidShutdownTimer.start();
#endif
              Serial.println(F("sys | tag setup canceled"));
              nfcTag.assignedFolder = 0;
              nfcTag.playbackMode = 0;
              nfcTag.assignedTrack = 0;
              mfrc522.PICC_HaltA();
              mfrc522.PCD_StopCrypto1();
              mp3.playMp3FolderTrack(msgSetupNewTagCancel);
              return;
            }
            // button 1 (right) hold for 2 sec or ir remote up: jump 10 tracks forward
            else if (inputEvent == B1H || inputEvent == IRU) {
              nfcTag.assignedTrack = min(nfcTag.assignedTrack + 10, 255);
              Serial.print(F("sys |     track "));
              Serial.println(nfcTag.assignedTrack);
              mp3.playMp3FolderTrack(nfcTag.assignedTrack);
              waitPlaybackToFinish();
              mp3.playFolderTrack(nfcTag.assignedFolder, nfcTag.assignedTrack);
            }
            // button 2 (left) hold for 2 sec or ir remote down: jump 10 tracks backwards
            else if (inputEvent == B2H || inputEvent == IRD) {
              nfcTag.assignedTrack = max(nfcTag.assignedTrack - 10, 1);
              Serial.print(F("sys |     track "));
              Serial.println(nfcTag.assignedTrack);
              mp3.playMp3FolderTrack(nfcTag.assignedTrack);
              waitPlaybackToFinish();
              mp3.playFolderTrack(nfcTag.assignedFolder, nfcTag.assignedTrack);
            }
#if defined(STATUSLED)
            blinkStatusLed(ledBlinkInterval);
#endif
            mp3.loop();
          }
          while (!setAssignedTrack);
          delay(500);
          Serial.print(F("sys |     track "));
          Serial.print(nfcTag.assignedTrack);
          Serial.println(F(" selected"));
        }
        switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
        cubiekidShutdownTimer.start();
#endif
        // save data to tag
        Serial.println(F("sys | attempting to save data to tag"));
        uint8_t bytesToWrite[] = {0x13, 0x37, 0xb3, 0x47,            // 0x1337 0xb347 magic cookie to identify our nfc tags
                                  0x01,                              // version 1
                                  nfcTag.assignedFolder,             // the folder selected by the user
                                  nfcTag.playbackMode,               // the playback mode selected by the user
                                  nfcTag.assignedTrack,              // the track selected by the user
                                  0x00, 0x00, 0x00, 0x00,            // reserved for future use
                                  0x00, 0x00, 0x00, 0x00             // reserved for future use
                                 };
#if defined(DBG)
        // for debug purposes, print the 16 bytes we are going write to the nfc tag
        Serial.print(F("dbg |"));
        for (uint8_t i = 0; i < 16; i++) {
          Serial.print(bytesToWrite[i] < 0x10 ? " 0" : " ");
          Serial.print(bytesToWrite[i], HEX);
        }
        Serial.println();
#endif
        uint8_t writeNfcTagStatus = writeNfcTagData(bytesToWrite, sizeof(bytesToWrite));
        // handle return codes from events that happened during writing to the nfc tag
        switch (writeNfcTagStatus) {
          case 1:
            Serial.println(F("sys | saving data to tag successfull"));
            mp3.playMp3FolderTrack(msgSetupNewTagConfirm);
            break;
          case 253:
            Serial.println(F("nfc | write error"));
            mp3.playMp3FolderTrack(msgSetupNewTagError);
            break;
          case 254:
            Serial.println(F("nfc | authentication failed"));
            mp3.playMp3FolderTrack(msgSetupNewTagError);
            break;
          case 255:
            Serial.println(F("nfc | tag is not supported"));
            mp3.playMp3FolderTrack(msgSetupNewTagError);
            break;
          default:
            Serial.println(F("nfc | unknown error"));
            mp3.playMp3FolderTrack(msgSetupNewTagError);
            break;
        }
        inputEvent = NOACTION;
      }

      // nfc tag is not blank but unknown, ignore
      else Serial.println(F("nfc | tag is not one of ours"));
      // # end - nfc tag does not have our magic cookie 0x1337 0xb347 on it (0)
      // ######################################################################
    }
    // # end - nfc tag is successfully read
    // ####################################

    // handle errors that happened during reading from the nfc tag
    else if (readNfcTagStatus == 253) Serial.println(F("nfc | read error"));
    else if (readNfcTagStatus == 254) Serial.println(F("nfc | authentication failed"));
    else if (readNfcTagStatus == 255) Serial.println(F("nfc | tag is not supported"));
    else Serial.println(F("nfc | unknown error"));
  }
  // # end - main code block
  // #######################

  // ##################################################################################
  // # handle button and ir remote events during playback or while waiting for nfc tags
  // ir remote center: toggle box lock
  if (inputEvent == IRC) {
    if (!isLocked) {
      Serial.println(F("sys | box locked"));
      isLocked = true;
#if defined(STATUSLED)
      burstStatusLed();
#endif
    }
    else {
      Serial.println(F("sys | box unlocked"));
      isLocked = false;
#if defined(STATUSLED)
      burstStatusLed();
#endif
    }
  }
  // button 0 (middle) press or ir remote play+pause: toggle playback
  else if ((inputEvent == B0P && !isLocked) || inputEvent == IRP) {
    if (isPlaying) {
      switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
      cubiekidShutdownTimer.start();
#endif
      Serial.println(F("sys | pause"));
      mp3.pause();
      // if the current playback mode is story book mode: store the current progress
      if (nfcTag.playbackMode == 5) {
        Serial.print(F("mp3 | story book mode > folder "));
        Serial.print(nfcTag.assignedFolder);
        Serial.print(F(" > track "));
        Serial.print(playback.playTrack);
        Serial.print(F("/"));
        Serial.print(playback.folderTrackCount);
        Serial.println(F(" > progress saved"));
        EEPROM.update(nfcTag.assignedFolder, playback.playTrack);
      }
    }
    else {
      if (playback.queueMode) {
        switchButtonConfiguration(PLAY);
#if defined(CUBIEKID)
        cubiekidShutdownTimer.stop();
#endif
        Serial.println(F("sys | play"));
        mp3.start();
      }
    }
  }
  // button 1 (right) press or ir remote up while playing: increase volume
  else if (((inputEvent == B1P && !isLocked) || inputEvent == IRU) && isPlaying) {
    uint8_t mp3CurrentVolume = mp3.getVolume();
    if (mp3CurrentVolume < mp3MaxVolume) {
      Serial.print(F("sys | increase volume to "));
      Serial.println(mp3CurrentVolume + 1);
      mp3.increaseVolume();
    }
    else {
      Serial.print(F("sys | maximum vomlume is "));
      Serial.println(mp3MaxVolume);
    }
  }
  // button 2 (left) press or ir remote down while playing: decrease volume
  else if (((inputEvent == B2P && !isLocked) || inputEvent == IRD) && isPlaying) {
    uint8_t mp3CurrentVolume = mp3.getVolume();
    if (mp3CurrentVolume > 0) {
      if (mp3CurrentVolume == 1) Serial.println(F("sys | mute"));
      else {
        Serial.print(F("sys | decrease volume to "));
        Serial.println(mp3CurrentVolume - 1);
      }
      mp3.decreaseVolume();
    }
    else Serial.println(F("sys | mute"));
  }
  // button 1 (right) hold for 2 sec or ir remote right, only during album, party and story book mode while playing: next track
  else if (((inputEvent == B1H && !isLocked) || inputEvent == IRR) && (nfcTag.playbackMode == 2 || nfcTag.playbackMode == 3 || nfcTag.playbackMode == 5) && isPlaying) {
    Serial.println(F("sys | next track"));
    playNextTrack(random(65536), true, true);
  }
  // button 2 (left) hold for 2 sec or ir remote left, only during album and story book mode while playing: previous track
  else if (((inputEvent == B2H && !isLocked) || inputEvent == IRL) && (nfcTag.playbackMode == 2 || nfcTag.playbackMode == 5) && isPlaying) {
    Serial.println(F("sys | previous track"));
    playNextTrack(random(65536), false, true);
  }
  // button 0 (middle) hold for 5 sec or ir remote menu, only during story book mode while playing: reset progress
  else if (((inputEvent == B0H && !isLocked) || inputEvent == IRM) && nfcTag.playbackMode == 5 && isPlaying) {
    Serial.print(F("mp3 | story book mode > folder "));
    Serial.print(nfcTag.assignedFolder);
    Serial.println(F(" > progress reset"));
    EEPROM.update(nfcTag.assignedFolder, 0);
    Serial.print(F("mp3 | story book mode > folder "));
    Serial.print(nfcTag.assignedFolder);
    Serial.print(F(" > track 1/"));
    Serial.println(playback.folderTrackCount);
    mp3.playFolderTrack(nfcTag.assignedFolder, playback.playTrack = 1);
  }
  // button 0 (middle) hold for 5 sec or ir remote menu while not playing: erase nfc tag
  else if (((inputEvent == B0H && !isLocked) || inputEvent == IRM) && !isPlaying) {
    switchButtonConfiguration(CONFIG);
#if defined(CUBIEKID)
    cubiekidShutdownTimer.stop();
#endif
    playback.queueMode = false;
    uint8_t writeNfcTagStatus = 0;
    Serial.println(F("sys | waiting for tag to erase"));
    mp3.playMp3FolderTrack(msgEraseTag);
    do {
      checkForInput();
      // button 0 (middle) hold for 2 sec or ir remote menu: cancel erase nfc tag
      if (inputEvent == B0H || inputEvent == IRM) {
        switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
        cubiekidShutdownTimer.start();
#endif
        Serial.println(F("sys | erasing tag canceled"));
        mp3.playMp3FolderTrack(msgEraseTagCancel);
        return;
      }
      // wait for nfc tag, erase once detected
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        switchButtonConfiguration(PAUSE);
#if defined(CUBIEKID)
        cubiekidShutdownTimer.start();
#endif
        Serial.println(F("nfc | tag detected"));
        Serial.println(F("nfc | erasing tag"));
        uint8_t bytesToWrite[16];
        for (uint8_t i = 0; i < 16; i++) bytesToWrite[i] = 0x00;
        writeNfcTagStatus = writeNfcTagData(bytesToWrite, sizeof(bytesToWrite));
        // handle return codes from events that happened during erasing the nfc tag
        switch (writeNfcTagStatus) {
          case 1:
            Serial.println(F("nfc | tag erased"));
            mp3.playMp3FolderTrack(msgEraseTagConfirm);
            break;
          case 253:
            Serial.println(F("nfc | write error"));
            mp3.playMp3FolderTrack(msgEraseTagError);
            break;
          case 254:
            Serial.println(F("nfc | authentication failed"));
            mp3.playMp3FolderTrack(msgEraseTagError);
            break;
          case 255:
            Serial.println(F("nfc | tag is not supported"));
            mp3.playMp3FolderTrack(msgEraseTagError);
            break;
          default:
            Serial.println(F("nfc | unknown error"));
            mp3.playMp3FolderTrack(msgEraseTagError);
            break;
        }
      }
#if defined(STATUSLED)
      blinkStatusLed(ledBlinkInterval);
#endif
      mp3.loop();
    }
    while (!writeNfcTagStatus);
  }
  // # end - handle button or ir remote events during playback or while waiting for nfc tags
  // #######################################################################################

  mp3.loop();
}
