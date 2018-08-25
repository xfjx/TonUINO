#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <DFMiniMp3.h>
#include <EEPROM.h>

//DFPlayer Mini
SoftwareSerial mySoftwareSerial(2, 3); // RX, TX
uint16_t numTracksInFolder;
uint16_t track;

struct StoredCard {
  byte id[4];
  int folder;
  byte mode;
  byte reserved1; // Um in Zukunft noch weitere Optionen
  byte reserved2; // konfigurieren zu können
  byte reserved3; // reservieren wir einfach mal ein paar Integer
};

StoredCard myCard;
static void nextTrack();
bool foundCard = false;

// implement a notification class,
// its member methods will get called
//
class Mp3Notify
{
  public:
    static void OnError(uint16_t errorCode)
    {
      // see DfMp3_Error for code meaning
      Serial.println();
      Serial.print("Com Error ");
      Serial.println(errorCode);
    }
    static void OnPlayFinished(uint16_t track)
    {
      Serial.print("Track beendet");
      Serial.println(track);
      delay(100);
      nextTrack();
    }
    static void OnCardOnline(uint16_t code)
    {
      Serial.println(F("SD Karte online "));
    }
    static void OnCardInserted(uint16_t code)
    {
      Serial.println(F("SD Karte bereit "));
    }
    static void OnCardRemoved(uint16_t code)
    {
      Serial.println(F("SD Karte entfernt "));
    }
};

static DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(mySoftwareSerial);

// Leider kann das Modul keine Queue abspielen.
static void nextTrack() {
  if (foundCard == false)
    // Wenn eine neue Karte angelernt wird soll das Ende eines Tracks nicht verarbeitet werden
    return;

  if (myCard.mode == 1)
  {
    Serial.println(F("Hörspielmodus ist aktiv -> Strom sparen"));
    mp3.sleep();
  }
  if (myCard.mode == 2)
  {
    Serial.println(F("Albummodus ist aktiv -> nächster Track"));
    if (track != numTracksInFolder) {
      track = track + 1;
      mp3.playFolderTrack(myCard.folder, track);
    } else
      mp3.sleep();
  }
  if (myCard.mode == 3)
  {
    Serial.println(F("Party Modus ist aktiv -> zufälligen Track spielen"));
    track = random(1, numTracksInFolder + 1);
    mp3.playFolderTrack(myCard.folder, track);
  }
}

// MFRC522
#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522
uint8_t successRead;
byte readCard[4];   // Stores scanned ID read from RFID Module

const int buttonPause = A0;
const int buttonUp = A1;
const int buttonDown = A2;

uint8_t numberOfCards = 0;

void setup() {

  Serial.begin(115200); // Es gibt ein paar Debug Ausgaben über die serielle Schnittstelle
  randomSeed(analogRead(A0));  // Zufallsgenerator initialisieren

  Serial.println(F("DIY NFC JUKEBOX"));
  Serial.println(F("BY THORSTEN VOß"));

  // Knöpfe mit PullUp
  pinMode(buttonPause, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);

  // NFC Leser initialisieren
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader

  // DFPlayer Mini initialisieren
  mp3.begin();
  mp3.setVolume(10);

  // RESET --- ALLE DREI KNÖPFE BEIM STARTEN GEDRÜCKT HALTEN -> alle bekannten Karten werden gelöscht
  if (digitalRead(buttonPause) == LOW && digitalRead(buttonUp) == LOW && digitalRead(buttonDown) == LOW) {
    Serial.println(F("Reset -> EEPROM wird gelöscht"));
    for (int i = 0 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 0);
    }
  }

  // Anzahl bekannter Karten auslesen
  numberOfCards = EEPROM.read(0);
}

void loop() {
  do {
    successRead = getID();
    mp3.loop();

    if (digitalRead(buttonPause) == LOW) {
      Serial.println(F("Play/Pause"));

      if (mp3.getStatus() == 513)
        mp3.pause();
      else
        mp3.start();

      delay(500);
    }
    if (digitalRead(buttonUp) == LOW) {
      Serial.println(F("Volume Up"));
      mp3.increaseVolume();
    }
    if (digitalRead(buttonDown) == LOW) {
      Serial.println(F("Volume Down"));
      mp3.decreaseVolume();
    }

  }
  while (!successRead);

  foundCard = false;

  for (int x = 0; x < numberOfCards; x++) {
    EEPROM.get(sizeof(StoredCard) * x + sizeof(int), myCard);
    if (checkTwo(readCard, myCard.id))
    {
      foundCard = true;
      numTracksInFolder = mp3.getFolderTrackCount(myCard.folder);

      // Hörspielmodus: eine zufällige Datei aus dem Ordner
      if (myCard.mode == 1) {
        Serial.println(F("Hörspielmodus -> zufälligen Track wiedergeben"));
        track = random(1, numTracksInFolder + 1);
        Serial.println(track);
        mp3.playFolderTrack(myCard.folder, track);
      }
      // Album Modus: kompletten Ordner spielen
      if (myCard.mode == 2) {
        Serial.println(F("Album Modus -> kompletten Ordner wiedergeben"));
        track = 1;
        mp3.playFolderTrack(myCard.folder, track);
      }
      // Party Modus: Ordner in zufälliger Reihenfolge
      if (myCard.mode == 3) {
        Serial.println(F("Party Modus -> Ordner in zufälliger Reihenfolge wiedergeben"));
        track = random(1, numTracksInFolder + 1);
        mp3.playFolderTrack(myCard.folder, track);
      }
      break;
    }
  }

  // Neue Karte konfigurieren
  if (foundCard == false) {
    Serial.print(F("Neue Karte konfigurieren"));

    for (int i = 0; i < 4; i++)
      myCard.id[i] = readCard[i];

    myCard.folder = 0;
    myCard.mode = 0;
    bool done = false;
    mp3.playMp3FolderTrack(100);
    do {
      if (digitalRead(buttonPause) == LOW) {
        done = true;
        delay(1000);
      }
      if (digitalRead(buttonUp) == LOW) {
        myCard.folder = min(myCard.folder + 1, 99);
        //mp3.playMp3FolderTrack(myCard.folder);
        mp3.playFolderTrack(myCard.folder, 1);
        delay(1000);
      }
      if (digitalRead(buttonDown) == LOW) {
        myCard.folder = max(myCard.folder - 1, 1);
        //mp3.playMp3FolderTrack(myCard.folder);
        mp3.playFolderTrack(myCard.folder, 1);
        delay(1000);
      }
    }
    while (done == false);

    done = false;
    mp3.playMp3FolderTrack(101);
    do {
      if (digitalRead(buttonPause) == LOW) {
        done = true;
        delay(1000);
      }
      if (digitalRead(buttonUp) == LOW) {
        myCard.mode = min(myCard.mode + 1, 3);
        mp3.playMp3FolderTrack(101 + myCard.mode);
        delay(1000);
      }
      if (digitalRead(buttonDown) == LOW) {
        myCard.mode = max(myCard.mode - 1, 1);
        mp3.playMp3FolderTrack(101 + myCard.mode);
        delay(1000);
      }
    }
    while (done == false);
    mp3.playMp3FolderTrack(110);
    EEPROM.put(sizeof(StoredCard) * numberOfCards + sizeof(int), myCard);
    numberOfCards = numberOfCards + 1;
    EEPROM.put(0, numberOfCards);
  }
}



///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}



///////////////////////////////////////// Check Bytes   ///////////////////////////////////
boolean checkTwo ( byte a[], byte b[] ) {
  boolean match = false;          // initialize card match to false
  if ( a[0] != 0 )      // Make sure there is something in the array first
    match = true;       // Assume they match at first
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] )     // IF a != b then set match = false, one fails, all fail
      match = false;
  }
  if ( match ) {      // Check to see if if match is still true
    return true;      // Return true
  }
  else  {
    return false;       // Return false
  }
}


