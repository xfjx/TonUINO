#include "chip_card.hpp"

#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>

#include "mp3.hpp"
#include "buttons.hpp"
#include "constants.hpp"
#include "logger.hpp"

namespace {
/**
  Helper routine to dump a byte array as hex values to Serial.
*/
String dump_byte_array(byte * buffer, byte bufferSize) {
  String res((char *)0);
  res.reserve(3*bufferSize);
  for (byte i = 0; i < bufferSize; i++) {
    res += String(buffer[i] < 0x10 ? " 0" : " ");
    res += String(buffer[i], HEX);
  }
  return res;
}

MFRC522::MIFARE_Key key{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
const byte sector       = 1;
const byte trailerBlock = 7;

const unsigned int removeDelay = 3;
} // namespace

Chip_card::Chip_card(Mp3 &mp3, Buttons &buttons)
: mfrc522(*(new MFRC522(mfrc522_SSPin, mfrc522_RSTPin)))
, mp3(mp3)
, buttons(buttons)
, cardRemovedSwitch(removeDelay)
{}

bool Chip_card::readCard(nfcTagObject &nfcTag) {
  // Show some details of the PICC (that is: the tag/card)
  LOG(card_log, s_info, F("Card UID: "), dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  LOG(card_log, s_info, F("PICC type: "), mfrc522.PICC_GetTypeName(piccType));

  byte buffer[18];
  MFRC522::StatusCode status = MFRC522::STATUS_ERROR;

  // Authenticate using key A
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K  ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K  ) )
  {
    LOG(card_log, s_info, F("Auth Classic using key A..."));
    status = mfrc522.PCD_Authenticate(
               MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the tempCard

    // Authenticate using key A
    LOG(card_log, s_info, F("Auth MIFARE UL..."));
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK) {
    LOG(card_log, s_error, F("PCD_Auth() failed: "), mfrc522.GetStatusCodeName(status));
    return false;
  }

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
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(4, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      LOG(card_log, s_error, F("MIFARE_Read() failed: "), mfrc522.GetStatusCodeName(status));
      return false;
    }
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte buffer2[18];
    byte size2 = sizeof(buffer2);

    status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Read(8, buffer2, &size2));
    if (status != MFRC522::STATUS_OK) {
      LOG(card_log, s_error, F("MIFARE_Read_1() failed: "), mfrc522.GetStatusCodeName(status));
      return false;
    }
    memcpy(buffer, buffer2, 4);

    status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Read(9, buffer2, &size2));
    if (status != MFRC522::STATUS_OK) {
      LOG(card_log, s_error, F("MIFARE_Read_2() failed: "), mfrc522.GetStatusCodeName(status));
      return false;
    }
    memcpy(buffer + 4, buffer2, 4);

    status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Read(10, buffer2, &size2));
    if (status != MFRC522::STATUS_OK) {
      LOG(card_log, s_error, F("MIFARE_Read_3() failed: "), mfrc522.GetStatusCodeName(status));
      return false;
    }
    memcpy(buffer + 8, buffer2, 4);

    status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Read(11, buffer2, &size2));
    if (status != MFRC522::STATUS_OK) {
      LOG(card_log, s_error, F("MIFARE_Read_4() failed: "), mfrc522.GetStatusCodeName(status));
      return false;
    }
    memcpy(buffer + 12, buffer2, 4);
  }

  LOG(card_log, s_info, F("Data on Card: "), dump_byte_array(buffer, 16));

  uint32_t tempCookie;
  tempCookie  = (uint32_t)buffer[0] << 24;
  tempCookie += (uint32_t)buffer[1] << 16;
  tempCookie += (uint32_t)buffer[2] <<  8;
  tempCookie += (uint32_t)buffer[3];

  nfcTag.cookie                     = tempCookie;
  nfcTag.version                    = buffer[4];
  nfcTag.nfcFolderSettings.folder   = buffer[5];
  nfcTag.nfcFolderSettings.mode     = static_cast<mode_t>(buffer[6]);
  nfcTag.nfcFolderSettings.special  = buffer[7];
  nfcTag.nfcFolderSettings.special2 = buffer[8];

  return true;
}

bool Chip_card::writeCard(const nfcTagObject &nfcTag) {
  MFRC522::PICC_Type mifareType;
  byte buffer[16] = {0x13, 0x37, 0xb3, 0x47,                              // 0x1337 0xb347 magic cookie to
                                                                          // identify our nfc tags
                     0x02,                                                // version 1
                     nfcTag.nfcFolderSettings.folder,                     // the folder picked by the user
                     static_cast<uint8_t>(nfcTag.nfcFolderSettings.mode), // the playback mode picked by the user
                     nfcTag.nfcFolderSettings.special,                    // track or function for admin cards
                     nfcTag.nfcFolderSettings.special2,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                    };

  mifareType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  MFRC522::StatusCode status = MFRC522::STATUS_ERROR;

  // Authenticate using key B
  //authentificate with the card and set card specific parameters
  if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
    LOG(card_log, s_info, F("Auth again using key A..."));
    status = mfrc522.PCD_Authenticate(
               MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  }
  else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the NFCtag

    // Authenticate using key A
    LOG(card_log, s_info, F("Auth UL..."));
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK) {
    LOG(card_log, s_error, F("PCD_Auth() failed: "), mfrc522.GetStatusCodeName(status));
    return false;
  }

  // Write data to the block
  LOG(card_log, s_info, F("Writing data: "), dump_byte_array(buffer, 16));

  if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(4, buffer, 16);
  }
  else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte buffer2[16];
    byte size2 = sizeof(buffer2);

    memset(buffer2, 0, size2);
    memcpy(buffer2, buffer, 4);
    status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Write(8, buffer2, 16));

    memset(buffer2, 0, size2);
    memcpy(buffer2, buffer + 4, 4);
    status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Write(9, buffer2, 16));

    memset(buffer2, 0, size2);
    memcpy(buffer2, buffer + 8, 4);
    status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Write(10, buffer2, 16));

    memset(buffer2, 0, size2);
    memcpy(buffer2, buffer + 12, 4);
    status = static_cast<MFRC522::StatusCode>(mfrc522.MIFARE_Write(11, buffer2, 16));
  }

  if (status != MFRC522::STATUS_OK) {
    LOG(card_log, s_error, F("MIFARE_Write() failed: "), mfrc522.GetStatusCodeName(status));
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
	mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader
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
    cardRemovedSwitch.reset();
    mfrc522.PICC_ReadCardSerial();
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

void Chip_card::waitForCardRemoved() {
  stopCrypto1();
  while (!cardRemoved) {
    getCardEvent();
  }
}

void Chip_card::waitForCardInserted() {
  mp3.playMp3FolderTrack(mp3Tracks::t_800_waiting_for_card);
  do {
    if (buttons.isBreak()) {
      mp3.playMp3FolderTrack(mp3Tracks::t_802_reset_aborted);
      return;
    }
    getCardEvent();
  } while (cardRemoved);

}


