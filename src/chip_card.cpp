#include "chip_card.hpp"

#include <Arduino.h>
#include <SPI.h>

#include "mp3.hpp"
#include "constants.hpp"
#include "logger.hpp"

// select whether StatusCode and PiccType are printed as names
// that uses about 690 bytes or 2.2% of flash
constexpr bool verbosePrintStatusCode = true;
constexpr bool verbosePrintPiccType   = false;

namespace {

constexpr size_t buffferSizeRead  = 18; // buffer size for read and write
constexpr size_t buffferSizeWrite = 16; // buffer size for read and write

const __FlashStringHelper* str_failed      () { return F(" failed: ") ; }
const __FlashStringHelper* str_MIFARE_Read () { return F("MIFARE_Read ") ; }
const __FlashStringHelper* str_MIFARE_Write() { return F("MIFARE_Write ") ; }

/**
  Helper routine to dump a byte array as hex values to Serial.
*/
void u8toa_hex(uint8_t number, char *arr)
{
    int pos = 0;
    if (number < 16)
        arr[pos++] = '0';
    do {
        const int r = number % 16;
        arr[pos++] = (r > 9) ? (r - 10) + 'a' : r + '0';
        number /= 16;
    } while (number != 0);
}
const char* dump_byte_array(byte * buffer, size_t bufferSize) {
  static char ret[3*10+1];
  ret[0] = '\0';
  if (bufferSize > 10)
    return ret;
  size_t pos = 0;
  for (uint8_t i = 0; i < bufferSize; ++i) {
    ret[pos++] = ' ';
    u8toa_hex(buffer[i], &ret[pos]);
    pos +=2;
  }
  ret[pos] = '\0';
  return ret;
}

auto printStatusCode(MFRC522& mfrc522, MFRC522::StatusCode status) {
  if constexpr (verbosePrintStatusCode)
    return mfrc522.GetStatusCodeName(status);
  else
    return static_cast<byte>(status);
}

auto printPiccType(MFRC522& mfrc522, MFRC522::PICC_Type piccType) {
  if constexpr (verbosePrintPiccType)
    return mfrc522.PICC_GetTypeName(piccType);
  else
    return static_cast<byte>(piccType);
}


MFRC522::MIFARE_Key key{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
const byte sector       = 1;
const byte trailerBlock = 7;
} // namespace

Chip_card::Chip_card(Mp3 &mp3)
: mfrc522(mfrc522_SSPin, mfrc522_RSTPin)
, mp3(mp3)
, cardRemovedSwitch(cardRemoveDelay)
{}

bool Chip_card::auth(MFRC522::PICC_Type piccType) {
  MFRC522::StatusCode status = MFRC522::STATUS_ERROR;

  // Authenticate using key A
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K  ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K  ) )
  {
    LOG(card_log, s_info, F("Auth Classic"));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the tempCard

    // Authenticate using key A
    LOG(card_log, s_info, F("Auth UL"));
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK) {
    LOG(card_log, s_error, F("Auth failed: "), printStatusCode(mfrc522, status));
    return false;
  }

  return true;
}

bool Chip_card::readCard(nfcTagObject &nfcTag) {
  // Show some details of the PICC (that is: the tag/card)
  LOG(card_log, s_info, F("Card UID: "), dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size));
  const MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  LOG(card_log, s_info, F("PICC type: "), printPiccType(mfrc522, piccType));

  byte buffer[buffferSizeRead];
  MFRC522::StatusCode status = MFRC522::STATUS_ERROR;

  if (not auth(piccType))
    return false;

  // Show the whole sector as it currently is
  // LOG(card_log, s_info, F("Current data in sector:"));
  // mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  // Serial.println();

  // Read data from the block
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K  ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K  ) )
  {
    byte size = sizeof(buffer);
    status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Read(4, buffer, &size));
    if (status != MFRC522::STATUS_OK)
      LOG(card_log, s_debug, str_MIFARE_Read(), F("4"), str_failed(), printStatusCode(mfrc522, status));
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte buffer2[buffferSizeRead];

    for (byte block = 8, bufpos = 0; block <= 11; ++block, bufpos += 4) {
      byte size2 = sizeof(buffer2);
      status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Read(8, buffer2, &size2));
      if (status != MFRC522::STATUS_OK) {
        LOG(card_log, s_debug, str_MIFARE_Read(), block, str_failed(), printStatusCode(mfrc522, status));
        break;
      }
      memcpy(buffer+bufpos, buffer2, 4);
    }
  }
  stopCrypto1();

  if (status != MFRC522::STATUS_OK) {
    LOG(card_log, s_error, str_MIFARE_Read(), str_failed(), printStatusCode(mfrc522, status));
    return false;
  }

  LOG(card_log, s_info, F("Data on Card: "), dump_byte_array(buffer, 9));

  uint32_t tempCookie = 0;
  for (byte i = 0, shift = 24; i < 4; ++i, shift -= 8)
    tempCookie  += static_cast<uint32_t>(buffer[i]) << shift;

  if (tempCookie != cardCookie)
    return false;

  const uint32_t version            = buffer[4];
  if (version == cardVersion) {
    nfcTag.nfcFolderSettings.folder   = buffer[5];
    nfcTag.nfcFolderSettings.mode     = static_cast<mode_t>(buffer[6]);
    nfcTag.nfcFolderSettings.special  = buffer[7];
    nfcTag.nfcFolderSettings.special2 = buffer[8];
  }
  else {
    LOG(card_log, s_warning, F("Unknown version "), version);
    nfcTag.nfcFolderSettings.folder   = 0;
    nfcTag.nfcFolderSettings.mode     = mode_t::none;
  }
  return true;
}

bool Chip_card::writeCard(const nfcTagObject &nfcTag) {

  constexpr byte coockie_4 = (cardCookie & 0x000000ff) >>  0;
  constexpr byte coockie_3 = (cardCookie & 0x0000ff00) >>  8;
  constexpr byte coockie_2 = (cardCookie & 0x00ff0000) >> 16;
  constexpr byte coockie_1 = (cardCookie & 0xff000000) >> 24;
  byte buffer[buffferSizeWrite] = {coockie_1, coockie_2, coockie_3, coockie_4,          // 0x1337 0xb347 magic cookie to
                                                                                        // identify our nfc tags
                                   cardVersion,                                         // version 1
                                   nfcTag.nfcFolderSettings.folder,                     // the folder picked by the user
                                   static_cast<uint8_t>(nfcTag.nfcFolderSettings.mode), // the playback mode picked by the user
                                   nfcTag.nfcFolderSettings.special,                    // track or function for admin cards
                                   nfcTag.nfcFolderSettings.special2,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                                  };

  const MFRC522::PICC_Type mifareType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  if (not auth(mifareType))
    return false;

  MFRC522::StatusCode status = MFRC522::STATUS_ERROR;

  // Write data to the block
  LOG(card_log, s_info, F("Writing data: "), dump_byte_array(buffer, 9));

  if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(4, buffer, sizeof(buffer));
    if (status != MFRC522::STATUS_OK)
      LOG(card_log, s_debug, str_MIFARE_Write(), F("4"), str_failed(), printStatusCode(mfrc522, status));
  }
  else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte buffer2[buffferSizeWrite];
    memset(buffer2, 0, sizeof(buffer2));

    for (byte block = 8, bufpos = 0; block <= 11; ++block, bufpos += 4) {
      memcpy(buffer2, buffer+bufpos, 4);
      status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Write(block, buffer2, sizeof(buffer2)));
      if (status != MFRC522::STATUS_OK) {
        LOG(card_log, s_debug, str_MIFARE_Write(), block, str_failed(), printStatusCode(mfrc522, status));
        break;
      }
    }
  }
  stopCrypto1();

  if (status != MFRC522::STATUS_OK) {
    LOG(card_log, s_error, str_MIFARE_Write(), str_failed(), printStatusCode(mfrc522, status));
    return false;
  }
  return true;
}

void Chip_card::sleepCard() {
    mfrc522.PCD_AntennaOff();
    mfrc522.PCD_SoftPowerDown();
}

void Chip_card::initCard() {
  SPI.begin();        // Init SPI bus
	mfrc522.PCD_Init(); // Init MFRC522
	LOG_CODE(card_log, s_debug, mfrc522.PCD_DumpVersionToSerial()); // Show details of PCD - MFRC522 Card Reader
}

void Chip_card::stopCard() {
  mfrc522.PICC_HaltA();
}

void Chip_card::stopCrypto1() {
  mfrc522.PCD_StopCrypto1();
}

cardEvent Chip_card::getCardEvent() {
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);
  MFRC522::StatusCode result = mfrc522.PICC_RequestA(bufferATQA, &bufferSize);

  if(result != mfrc522.STATUS_OK) {
    ++cardRemovedSwitch;
  } else {
    mfrc522.PICC_ReadCardSerial();
    cardRemovedSwitch.reset();
  }

  if (cardRemovedSwitch.on()) {
    if (not cardRemoved) {
      LOG(card_log, s_info, F("Card Removed"));
      cardRemoved = true;
      stopCard();
      return cardEvent::removed;
    }
  }
  else {
    if (cardRemoved) {
      LOG(card_log, s_info, F("Card Inserted"));
      cardRemoved = false;
      return cardEvent::inserted;
    }
  }
  return cardEvent::none;
}

