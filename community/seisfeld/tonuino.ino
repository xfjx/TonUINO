/* -----------------------------------------------------------------------------------------
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

  Button B1 (pin A0): play+pause (middle button)
  Button B2 (pin A1): volume up (right button)
  Button B3 (pin A2): volume down (left button)

  button actions:
  ---------------

  Hold B2 or B3 for 2 seconds during playback in album mode: Skip to the next/previous track
  Hold B3 for 2 seconds during playback in party mode: Skip to the next track

  Hold both B2 and B3 for 2 seconds during idle: Enter erase nfc tag mode
  Hold both B2 and B3 for 2 seconds during erase nfc tag mode: Cancel erase nfc tag mode
  Hold both B2 and B3 for 2 seconds during nfc tag setup mode: Cancel nfc tag setup mode

  ir remote:
  ----------

  If a TSOP38238 is connected to pin 5, you can also use an ir remote to remote
  control the jukebox. There are code mappings for the silver apple remote below, but you can
  change them to other codes to match different remotes. This feature can be disabled by
  commenting a define below, to save some memory. The actions are as follows:

  During playback:
  play+pause / center - toggle playback
  up / down - volume up / down
  left / right - previous / next track during album mode, next track during party mode

  During idle:
  menu - enter erase nfc tag mode

  During erase nfc tag mode:
  menu - cancel

  During nfc tag setup mode:
  play+pause / center - confirm
  up / right - next option
  down / left - previous option
  menu - cancel

  status led:
  -----------

  If a led is connected to pin 6, limited status information is given using that led.
  The led is solid on when the jukebox is running (ie. has power and got initialized). The
  led is pulsing slowly to indicate playback. When the box is in setup new nfc tag or erase
  nfc tag mode, the led is blinking every 500ms. This feature can be disabled by commenting
  a define below, to save some memory.

  non standard libraries used in this sketch:
  -------------------------------------------

  MFRC522.h - https://github.com/miguelbalboa/rfid
  DFMiniMp3.h - https://github.com/Makuna/DFMiniMp3
  Bounce2.h - https://github.com/thomasfredericks/Bounce2
  IRremote.h - https://github.com/z3t0/Arduino-IRremote
*/

// comment below line to disable ir remote support, saves about 5.8 KiB of program memory
#define TSOP38238

// comment below line to disable status led support, saves about 530 Byte of program memory
#define STATUSLED

// include required libraries
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DFMiniMp3.h>
#include <Bounce2.h>

#if defined(TSOP38238)
#include <IRremote.h>
#endif


// define global constants
const uint8_t softwareSerialTxPin = 2;              // software serial tx, wired with 1k ohm to rx pin of DFPlayer Mini
const uint8_t softwareSerialRxPin = 3;              // software serial rx, wired straight to tx pin of DFPlayer Mini
const uint8_t mp3BusyPin = 4;                       // reports play state of DFPlayer Mini, low = playing
const uint8_t irReceiverPin = 5;                    // pin used for the ir receiver
const uint8_t statusLedPin = 6;                     // pin used for status led
const uint8_t nfcResetPin = 9;                      // used for spi communication to nfc module
const uint8_t nfcSlaveSelectPin = 10;               // used for spi communication to nfc module
const uint8_t rngSeedPin = A5;                      // used to seed the random number generator, should be unconnected
const uint8_t buPins[] = { A0, A1, A2 };            // pins used for the buttons
const uint8_t buCount = sizeof(buPins);             // number of buttons
const uint8_t mp3StartVolume = 10;                  // initial volume of DFPlayer Mini
const uint8_t mp3MaxVolume = 25;                    // maximal volume of DFPlayer Mini
const uint16_t buHoldTime = 2000;                   // button hold time, 2 seconds
const uint32_t debugConsoleSpeed = 115200;          // speed for the debug console

// define message to mp3 file mappings for spoken feedback
const uint8_t msgWelcome = 100;                     // 01
const uint8_t msgSetupNewTag = 101;                 // 02
const uint8_t msgSetupNewTagFolderAssigned = 102;   // 03
const uint8_t msgSetupNewTagStoryMode = 103;        // 04
const uint8_t msgSetupNewTagAlbumMode = 104;        // 05
const uint8_t msgSetupNewTagPartyMode = 105;        // 06
const uint8_t msgSetupNewTagConfirm = 110;          // 07
const uint8_t msgSetupNewTagError = 114;            // 08
const uint8_t msgEraseTag = 111;                    // 09
const uint8_t msgEraseTagConfirm = 112;             // 10
const uint8_t msgEraseTagError = 115;               // 11
const uint8_t msgCancel = 113;                      // 12
const uint8_t msgCount = 12;                        // used to calculate the total ammount of tracks on the sd card

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

// button states
enum { UNCHANGED, PUSH, RELEASE };                  // button states: UNCHANGED = 0, PUSH = 1, RELEASE = 2

// button actions
enum { NOACTION,                                    // NOACTION, 0
       B1P, B2P, B3P,                               // single button pushes, 1 -> buCount
       B1H, B2H, B3H,                               // single button holds, buCount + 1 -> 2 * buCount
       B23H,                                        // multi button holds and
       IRU, IRD, IRL, IRR, IRC, IRM, IRP            // ir remote events, 2 * buCount + 1 -> END
     };

// this object stores nfc tag data
struct nfcTagObject {
  uint8_t  returnCode;
  uint32_t cookie;
  uint8_t  version;
  uint8_t  assignedFolder;
  uint8_t  playbackMode;
} nfcTag;

// define global variables
uint8_t playTrack = 1;
uint16_t totalTrackCount = 0;
uint16_t folderTrackCount = 0;
bool initNfcTagPlayback = false;

// ############################################################ no variable definitions below this line ############################################################

// this function needs to be declared here for the first time because it's called in class Mp3Notify
// the function itself is down below
void playNextTrack(uint16_t globalTrack, bool directionForward, bool isInteractive);

// used by DFPlayer Mini library during callbacks
class Mp3Notify {
  public:
    static void OnError(uint16_t returnValue) {
      Serial.print(F("mp3 | error code "));
      Serial.println(returnValue);
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

Bounce bounce[buCount];                                                       // create Bounce instances
MFRC522 mfrc522(nfcSlaveSelectPin, nfcResetPin);                              // create MFRC522 instance
SoftwareSerial secondarySerial(softwareSerialRxPin, softwareSerialTxPin);     // create SoftwareSerial instance
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(secondarySerial);                    // create DFMiniMp3 instance

#if defined(TSOP38238)
IRrecv irReceiver(irReceiverPin);                                             // create IRrecv instance
decode_results irReadings;                                                    // create decode_results instance to store received ir readings
#endif

// checks for input from buttons and ir remote, debounces buttons and then returns an event
// button code is based on ideas from http://forum.arduino.cc/index.php?topic=296409.0
uint8_t checkInput() {
  static bool buDoEvent = false;                                              // was there an event for a single button? yes/no
  static bool buBlockEvent = false;                                           // block subsequent events? yes/no
  static uint8_t buEventCount = 0;                                            // track how many buttons are being pushed simultaniously
  static uint8_t buEvent = NOACTION;                                          // event to be executed
  static uint32_t buEventTime = 0;                                            // time when a button was pushed

  // if no button is pushed, reset
  if (!buEventCount) {
    buDoEvent = false;
    buBlockEvent = false;
    buEventCount = 0;
  }
  buEvent = NOACTION;

  // cycle through all buttons
  for (uint8_t i = 0; i < buCount; i++) {
    bounce[i].update();
    // button was pushed
    if (bounce[i].fell()) {
      if (buEventCount < buCount) buEventCount++;
      buEventTime = millis();
      if (!buDoEvent) {
        buDoEvent = true;
        buEvent = i + 1;
      }
    }
    // button was released
    else if (bounce[i].rose()) {
      if (buEventCount) buEventCount--;
      buBlockEvent = true;
    }
    // button hold events
    if (!buBlockEvent) {
      if (millis() - buEventTime >= buHoldTime) {
        // single button hold events
        if (!bounce[i].read() && buEventCount == 1) {
          buEvent = i + buCount + 1;
          buBlockEvent = true;
        }
        // multi button hold events
        if (!bounce[1].read() && !bounce[2].read() && buEventCount == 2) {
          buEvent = B23H;
          buBlockEvent = true;
        }
      }
    }
  }

#if defined(TSOP38238)
  // poll ir receiver, has precedence over physical buttons
  if (irReceiver.decode(&irReadings)) {
    switch (irReadings.value & 0xFFFF) {
      // button up
      case ir1ButtonUp:
      case ir2ButtonUp:
        buEvent = IRU;
        break;
      // button down
      case ir1ButtonDown:
      case ir2ButtonDown:
        buEvent = IRD;
        break;
      // button left
      case ir1ButtonLeft:
      case ir2ButtonLeft:
        buEvent = IRL;
        break;
      // button right
      case ir1ButtonRight:
      case ir2ButtonRight:
        buEvent = IRR;
        break;
      // button center
      case ir1ButtonCenter:
      case ir2ButtonCenter:
        buEvent = IRC;
        break;
      // button menu
      case ir1ButtonMenu:
      case ir2ButtonMenu:
        buEvent = IRM;
        break;
      // button play+pause
      case ir1ButtonPlayPause:
      case ir2ButtonPlayPause:
        buEvent = IRP;
        break;
      default:
        break;
    }
    irReceiver.resume();
  }
#endif

  return buEvent;
}

// plays next track depending on current playback mode
void playNextTrack(uint16_t globalTrack, bool directionForward, bool isInteractive) {
  static uint16_t lastCallTrack = 0;

  //delay 100ms to be on the safe side with the serial communication
  delay(100);

  // we don't advance to a new track on interactive voice feedback (ie. during configuration of a new nfc tag)
  if (!initNfcTagPlayback) return;

  // story mode: play one random track in folder
  // there is no next track in story mode -> stop playback
  if (nfcTag.playbackMode == 1) {
    Serial.println(F("mp3 | story mode -> stop"));
    mp3.stop();
  }

  // album mode: play complete folder
  // advance to the next track, stop if the end of the folder is reached or go back to the previous track
  if (nfcTag.playbackMode == 2) {

    // **workaround for some DFPlayer mini modules that make two callbacks in a row when finishing a track**
    // check if we get called with the same track number twice in a row, if yes return immediately
    if (lastCallTrack == globalTrack) return;
    else lastCallTrack = globalTrack;

    // play next track?
    if (directionForward) {
      // there are more tracks after the current one, play next track
      if (playTrack < folderTrackCount) {
        playTrack++;
        Serial.print(F("mp3 | album mode -> folder "));
        Serial.print(nfcTag.assignedFolder);
        Serial.print(F(" -> track "));
        Serial.print(playTrack);
        Serial.print(F(" of "));
        Serial.println(folderTrackCount);
        mp3.playFolderTrack(nfcTag.assignedFolder, playTrack);
      }
      // there are no more tracks after the current one
      else {
        // user wants to manually play the next track, ignore the next track command
        if (isInteractive) Serial.println(F("mp3 | album mode -> end of folder -> ignore next track"));
        // stop playback
        else {
          Serial.println(F("mp3 | album mode -> end of folder -> stop"));
          mp3.stop();
        }
      }
    }
    // play previous track?
    else {
      // there are more tracks before the current one, play the previous track
      if (playTrack > 1) {
        playTrack--;
        Serial.print(F("mp3 | album mode -> folder "));
        Serial.print(nfcTag.assignedFolder);
        Serial.print(F(" -> track "));
        Serial.print(playTrack);
        Serial.print(F(" of "));
        Serial.println(folderTrackCount);
        mp3.playFolderTrack(nfcTag.assignedFolder, playTrack);
      }
      // there are no more tracks before the current one, ignore the previous track command
      else Serial.println(F("mp3 | album mode -> beginning of folder -> ignore previous track"));
    }
  }

  // party mode: shuffle tracks in folder indefinitely
  // just play another random track
  if (nfcTag.playbackMode == 3) {
    playTrack = random(1, folderTrackCount + 1);
    Serial.print(F("mp3 | party mode -> folder "));
    Serial.print(nfcTag.assignedFolder);
    Serial.print(F(" -> track "));
    Serial.print(playTrack);
    Serial.print(F(" of "));
    Serial.println(folderTrackCount);
    mp3.playFolderTrack(nfcTag.assignedFolder, playTrack);
  }
}

// reads data from nfc tag
nfcTagObject readNfcTagData() {
  nfcTagObject nfcTag;
  uint8_t mifareBlock = 4;
  uint8_t mifareData[18];
  uint8_t mifareDataSize = sizeof(mifareData);
  MFRC522::StatusCode mifareStatus;
  MFRC522::MIFARE_Key mifareKey;
  MFRC522::PICC_Type mifareType;
  for (uint8_t i = 0; i < 6; i++) mifareKey.keyByte[i] = 0xFF;

  nfcTag.returnCode = 0;
  nfcTag.cookie = 0;
  nfcTag.version = 0;
  nfcTag.assignedFolder = 0;
  nfcTag.playbackMode = 0;

  // check if card type is supported
  mifareType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (mifareType != MFRC522::PICC_TYPE_MIFARE_MINI && mifareType != MFRC522::PICC_TYPE_MIFARE_1K && mifareType != MFRC522::PICC_TYPE_MIFARE_4K) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    nfcTag.returnCode = 255;
    return nfcTag;
  }
  else {
    // check if we can authenticate with mifareKey
    mifareStatus = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, mifareBlock, &mifareKey, &(mfrc522.uid));
    if (mifareStatus != MFRC522::STATUS_OK) {
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      nfcTag.returnCode = 254;
      return nfcTag;
    }
    else {
      // read data from nfc tag
      mifareStatus = (MFRC522::StatusCode) mfrc522.MIFARE_Read(mifareBlock, mifareData, &mifareDataSize);
      if (mifareStatus != MFRC522::STATUS_OK) {
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        nfcTag.returnCode = 253;
        return nfcTag;
      }
      else {
        // convert 4 byte cookie to 32bit decimal for easier handling
        uint32_t tempCookie;
        tempCookie  = (uint32_t) mifareData[0] << 24;
        tempCookie += (uint32_t) mifareData[1] << 16;
        tempCookie += (uint32_t) mifareData[2] << 8;
        tempCookie += (uint32_t) mifareData[3];

        // if cookie is not blank return data
        if (tempCookie != 0) {
          mfrc522.PICC_HaltA();
          mfrc522.PCD_StopCrypto1();
          nfcTag.returnCode = 1;
          nfcTag.cookie = tempCookie;
          nfcTag.version = mifareData[4];
          nfcTag.assignedFolder = mifareData[5];
          nfcTag.playbackMode = mifareData[6];
          return nfcTag;
        }
        else {
          nfcTag.returnCode = 1;
          nfcTag.cookie = 0;
          return nfcTag;
        }
      }
    }
  }
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
  if (mifareType != MFRC522::PICC_TYPE_MIFARE_MINI && mifareType != MFRC522::PICC_TYPE_MIFARE_1K && mifareType != MFRC522::PICC_TYPE_MIFARE_4K) nfcTag.returnCode = 255;
  else {
    // check if we can authenticate with mifareKey
    mifareStatus = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, mifareTrailerBlock, &mifareKey, &(mfrc522.uid));
    if (mifareStatus != MFRC522::STATUS_OK) returnCode = 254;
    else {
      // write data to nfc tag
      mifareStatus = (MFRC522::StatusCode) mfrc522.MIFARE_Write(mifareBlock, mifareData, mifareDataSize);
      if (mifareStatus != MFRC522::STATUS_OK) returnCode = 253;
      else returnCode = 1;
    }
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  return returnCode;
}

#if defined(STATUSLED)
// fade in/out status led during playback, on idle set to full brightness
void fadeStatusLed(bool isPlaying) {
  static bool statusLedDirection;
  static int16_t statusLedValue = 255;
  static uint32_t statusLedOldMillis;

  // jukebox is playing, fade status led in or out
  if (isPlaying) {
    uint32_t statusLedNewMillis = millis();
    if (statusLedNewMillis - statusLedOldMillis >= 100) {
      statusLedOldMillis = statusLedNewMillis;
      if (statusLedDirection) {
        statusLedValue = statusLedValue + 10;
        if (statusLedValue >= 255) {
          statusLedValue = 255;
          statusLedDirection = !statusLedDirection;
        }
      }
      else {
        statusLedValue = statusLedValue - 10;
        if (statusLedValue <= 0) {
          statusLedValue = 0;
          statusLedDirection = !statusLedDirection;
        }
      }
      analogWrite(statusLedPin, statusLedValue);
    }
  }
  // jukebox is not playing, set to full brightness
  else {
    statusLedValue = 255;
    analogWrite(statusLedPin, statusLedValue);
  }
}

// blink status led every 500ms during setup new nfc tag and erase nfc tag
void blinkStatusLed() {
  static bool statusLedState;
  static uint32_t statusLedOldMillis;

  uint32_t statusLedNewMillis = millis();
  if (statusLedNewMillis - statusLedOldMillis >= 500) {
    statusLedOldMillis = statusLedNewMillis;
    statusLedState = !statusLedState;
    digitalWrite(statusLedPin, statusLedState);
  }
}
#endif

void setup()
{
  Serial.begin(debugConsoleSpeed);
  while (!Serial);
  Serial.println(F("sys | TonUINO JUKEBOX"));
  Serial.println(F("sys | BY THORSTEN VOß"));
  Serial.println(F("---------------------"));
  Serial.println(F("sys | initializing nfc module"));
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println(F("sys | initializing mp3 module"));
  mp3.begin();
  delay(2000);
  Serial.print(F("sys |    volume: "));
  Serial.println(mp3StartVolume);
  mp3.setVolume(mp3StartVolume);
  Serial.print(F("sys |    tracks: "));
  totalTrackCount = mp3.getTotalTrackCount() - msgCount;
  Serial.println(totalTrackCount);
  pinMode(mp3BusyPin, INPUT);

  Serial.println(F("sys | initializing buttons"));
  for (uint8_t i = 0; i < buCount; i++) {
    pinMode(buPins[i], INPUT_PULLUP);
    bounce[i].attach(buPins[i]);
    bounce[i].interval(10);
  }

  Serial.println(F("sys | initializing rng"));
  randomSeed(analogRead(rngSeedPin));

#if defined(TSOP38238)
  Serial.println(F("sys | initializing ir receiver"));
  irReceiver.enableIRIn();
#endif

#if defined(STATUSLED)
  Serial.println(F("sys | initializing status led"));
  pinMode(statusLedPin, OUTPUT);
  digitalWrite(statusLedPin, HIGH);
#endif

  Serial.println(F("sys | system is ready"));
  mp3.playMp3FolderTrack(msgWelcome);
}

void loop()
{
  uint8_t inputEvent = checkInput();
  bool isPlaying = !digitalRead(mp3BusyPin);

  // ######################################################
  // # main code block, if nfc tag is detected do something
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println(F("nfc | tag detected"));
    nfcTag = readNfcTagData();
    // ##############################
    // # nfc tag is successfully read
    if (nfcTag.returnCode == 1) {
      Serial.println(F("nfc | successfully read tag"));
      // #######################################################################################################
      // # nfc tag has our magic cookie 0x1337 0xb347 on it (322417479), use data from nfc tag to start playback
      if (nfcTag.cookie == 322417479) {
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
          default:
            break;
        }
        // start playback
        Serial.println(F("sys | starting playback"));
        folderTrackCount = mp3.getFolderTrackCount(nfcTag.assignedFolder);
        switch (nfcTag.playbackMode) {
          // story mode
          case 1:
            playTrack = random(1, folderTrackCount + 1);
            Serial.print(F("mp3 | story mode -> randomly play one of "));
            Serial.print(folderTrackCount);
            Serial.print(F(" tracks in folder "));
            Serial.println(nfcTag.assignedFolder);
            Serial.print(F("mp3 | story mode -> folder "));
            Serial.print(nfcTag.assignedFolder);
            Serial.print(F(" -> track "));
            Serial.print(playTrack);
            Serial.print(F(" of "));
            Serial.println(folderTrackCount);
            break;
          // album mode
          case 2:
            playTrack = 1;
            Serial.print(F("mp3 | album mode -> sequentially play all "));
            Serial.print(folderTrackCount);
            Serial.print(F(" tracks in folder "));
            Serial.println(nfcTag.assignedFolder);
            Serial.print(F("mp3 | album mode -> folder "));
            Serial.print(nfcTag.assignedFolder);
            Serial.print(F(" -> track "));
            Serial.print(playTrack);
            Serial.print(F(" of "));
            Serial.println(folderTrackCount);
            break;
          // party mode
          case 3:
            playTrack = random(1, folderTrackCount + 1);
            Serial.print(F("mp3 | party mode -> shuffle all "));
            Serial.print(folderTrackCount);
            Serial.print(F(" tracks in folder "));
            Serial.println(nfcTag.assignedFolder);
            Serial.print(F("mp3 | party mode -> folder "));
            Serial.print(nfcTag.assignedFolder);
            Serial.print(F(" -> track "));
            Serial.print(playTrack);
            Serial.print(F(" of "));
            Serial.println(folderTrackCount);
            break;
          default:
            break;
        }
        initNfcTagPlayback = true;
        mp3.playFolderTrack(nfcTag.assignedFolder, playTrack);
      }
      // # end - nfc tag has our magic cookie 0x1337 0xb347 on it (322417479)
      // ####################################################################

      // #######################################################################################################
      // # nfc tag does not have our magic cookie 0x1337 0xb347 on it (0), start setup to configure this nfc tag
      else if (nfcTag.cookie == 0) {
        Serial.println(F("nfc | tag is blank"));
        Serial.println(F("sys | starting tag setup"));
        // let user pick the folder to assign
        initNfcTagPlayback = false;
        bool setAssignedFolder = false;
        Serial.println(F("sys |   pick folder"));
        mp3.playMp3FolderTrack(msgSetupNewTag);
        // loop until folder is assigned
        do {
          uint8_t inputEvent = checkInput();
          // button 1 (middle) single push or ir remote play+pause / center: confirm selected folder
          if (inputEvent == B1P || inputEvent == IRP || inputEvent == IRC) {
            if (nfcTag.assignedFolder == 0) {
              Serial.println(F("sys |     no folder selected"));
              continue;
            }
            else setAssignedFolder = true;
          }
          // button 2 (right) single push or ir remote up / right: next folder
          else if (inputEvent == B2P || inputEvent == IRU || inputEvent == IRR) {
            nfcTag.assignedFolder = min(nfcTag.assignedFolder + 1, 99);
            Serial.print(F("sys |     folder "));
            Serial.println(nfcTag.assignedFolder);
            mp3.playFolderTrack(nfcTag.assignedFolder, 1);
          }
          // button 3 (left) single push or ir remote down / left: previous folder
          else if (inputEvent == B3P || inputEvent == IRD || inputEvent == IRL) {
            nfcTag.assignedFolder = max(nfcTag.assignedFolder - 1, 1);
            Serial.print(F("sys |     folder "));
            Serial.println(nfcTag.assignedFolder);
            mp3.playFolderTrack(nfcTag.assignedFolder, 1);
          }
          // button 2 (right) & button 3 (left) multi hold for 2 sec or ir remote menu: cancel tag setup
          else if (inputEvent == B23H || inputEvent == IRM) {
            Serial.println(F("sys | tag setup canceled"));
            nfcTag.assignedFolder = 0;
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            mp3.playMp3FolderTrack(msgCancel);
            return;
          }

#if defined(STATUSLED)
          // blink status led
          blinkStatusLed();
#endif

          mp3.loop();
        }
        while (!setAssignedFolder);
        delay(500);
        Serial.print(F("sys |     folder "));
        Serial.print(nfcTag.assignedFolder);
        Serial.println(F(" selected"));
        // let user pick playback mode
        initNfcTagPlayback = false;
        bool setPlaybackMode = false;
        Serial.println(F("sys |   pick playback mode"));
        mp3.playMp3FolderTrack(msgSetupNewTagFolderAssigned);
        // loop until playback mode is set
        do {
          uint8_t inputEvent = checkInput();
          // button 1 (middle) single push or ir remote play+pause / center: confirm selected playback mode
          if (inputEvent == B1P || inputEvent == IRP || inputEvent == IRC) {
            if (nfcTag.playbackMode == 0) {
              Serial.println(F("sys |     no playback mode selected"));
              continue;
            }
            else setPlaybackMode = true;
          }
          // button 2 (right) single push or ir remote up / right: next playback mode
          else if (inputEvent == B2P || inputEvent == IRU || inputEvent == IRR) {
            nfcTag.playbackMode = min(nfcTag.playbackMode + 1, 3);
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
              default:
                break;
            }
          }
          // button 3 (left) single push or ir remote down / left: previous playback mode
          else if (inputEvent == B3P || inputEvent == IRD || inputEvent == IRL) {
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
              default:
                break;
            }
          }
          // button 2 (right) & button 3 (left) multi hold for 2 sec or ir remote menu: cancel tag setup
          else if (inputEvent == B23H || inputEvent == IRM) {
            Serial.println(F("sys | tag setup canceled"));
            nfcTag.playbackMode = 0;
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            mp3.playMp3FolderTrack(msgCancel);
            return;
          }

#if defined(STATUSLED)
          // blink status led
          blinkStatusLed();
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
          default:
            break;
        }
        Serial.println(F(" selected"));
        // save data to tag
        Serial.println(F("sys | attempting to save data to tag"));
        uint8_t bytesToWrite[] = { 0x13, 0x37, 0xb3, 0x47,            // 0x1337 0xb347 magic cookie to identify our nfc tags
                                   0x01,                              // version 1
                                   nfcTag.assignedFolder,             // the folder picked by the user
                                   nfcTag.playbackMode,               // the playback mode picked by the user
                                   0x00,                              // reserved for future use
                                   0x00, 0x00, 0x00, 0x00,            // reserved for future use
                                   0x00, 0x00, 0x00, 0x00             // reserved for future use
                                 };
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
      }
      // # end - nfc tag does not have our magic cookie 0x1337 0xb347 on it (0)
      // ######################################################################

      // nfc tag is unknown but not not blank, ignore
      else {
        Serial.println(F("nfc | tag is not one of ours"));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
      }
    }
    // # end - nfc tag is successfully read
    // ####################################

    // handle errors that happened during reading from the nfc tag
    else if (nfcTag.returnCode == 253) Serial.println(F("nfc | read error"));
    else if (nfcTag.returnCode == 254) Serial.println(F("nfc | authentication failed"));
    else if (nfcTag.returnCode == 255) Serial.println(F("nfc | tag is not supported"));
    else Serial.println(F("nfc | unknown error"));
  }
  // # end - main code block
  // #######################

  // ##################################################################################
  // # handle button and ir remote events during playback or while waiting for nfc tags
  // button 1 (middle) single push or ir remote play+pause / center: toggle playback
  if (inputEvent == B1P || inputEvent == IRP || inputEvent == IRC) {
    if (isPlaying) {
      Serial.println(F("sys | pause"));
      mp3.pause();
    }
    else {
      Serial.println(F("sys | play"));
      mp3.start();
    }
  }
  // button 2 (right) single push or ir remote up: increase volume
  else if ((inputEvent == B2P || inputEvent == IRU) && isPlaying) {
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
  // button 3 (left) single push or ir remote down: decrease volume
  else if ((inputEvent == B3P || inputEvent == IRD) && isPlaying) {
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
  // button 2 (right) single hold or ir remote right, only during album and party mode while playing: next track
  else if ((inputEvent == B2H || inputEvent == IRR) && (nfcTag.playbackMode == 2 || nfcTag.playbackMode == 3) && isPlaying) {
    Serial.println(F("sys | next track"));
    // bloody hack: decrease volume 1 step because it got increased before due to single button push action
    if (inputEvent == B2H) mp3.decreaseVolume();
    playNextTrack(random(65536), true, true);
  }
  // button 3 (left) single hold or ir remote left, only during album mode while playing: previous track
  else if ((inputEvent == B3H || inputEvent == IRL) && nfcTag.playbackMode == 2 && isPlaying) {
    Serial.println(F("sys | previous track"));
    // bloody hack: increase volume 1 step because it got decreased before due to single button push action
    if (inputEvent == B3H) mp3.increaseVolume();
    playNextTrack(random(65536), false, true);
  }
  // button 2 (right) & button 3 (left) multi hold for 2 sec or ir remote menu, only while not playing: erase nfc tag
  else if ((inputEvent == B23H || inputEvent == IRM) && !isPlaying) {
    initNfcTagPlayback = false;
    uint8_t writeNfcTagStatus = 0;
    Serial.println(F("sys | waiting for tag to erase"));
    mp3.playMp3FolderTrack(msgEraseTag);
    do {
      uint8_t inputEvent = checkInput();
      // button 2 (right) & button 3 (left) multi hold for 2 sec or ir remote menu: cancel erase nfc tag
      if (inputEvent == B23H || inputEvent == IRM) {
        Serial.println(F("sys | erasing tag canceled"));
        mp3.playMp3FolderTrack(msgCancel);
        return;
      }
      // wait for nfc tag, erase once detected
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        Serial.println(F("nfc | tag detected"));
        Serial.println(F("nfc | erasing tag"));
        uint8_t bytesToWrite[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
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
      // blink status led
      blinkStatusLed();
#endif

      mp3.loop();
    }
    while (!writeNfcTagStatus);
  }
  // # end - handle button or ir remote events during playback or while waiting for nfc tags
  // #######################################################################################

#if defined(STATUSLED)
  // fade status led
  fadeStatusLed(isPlaying);
#endif

  mp3.loop();
}
