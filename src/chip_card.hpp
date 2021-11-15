#ifndef SRC_CHIP_CARD_HPP_
#define SRC_CHIP_CARD_HPP_

#include <stdint.h>

const uint32_t cardCookie = 322417479;

enum class mode_t: uint8_t {
	none          =   0,

	// folder modes
	hoerspiel     =   1,
	album         =   2,
	party         =   3,
	einzel        =   4,
	hoerbuch      =   5,
	admin         =   6,
	hoerspiel_vb  =   7,
	album_vb      =   8,
	party_vb      =   9,

	// modifier modes
  sleep_timer   =   1,
  freeze_dance  =   2,
  locked        =   3,
  toddler       =   4,
  kindergarden  =   5,
  repeat_single =   6,

	admin_card    = 0xff,
};

struct folderSettings {
  uint8_t folder;
  mode_t  mode;
  uint8_t special;
  uint8_t special2;
};

// this object stores nfc tag data
struct nfcTagObject {
  uint32_t       cookie;
  uint8_t        version;
  folderSettings nfcFolderSettings;
};

class MFRC522; // forward declaration to not have to include it here

class delayedSwitchOn {
public:
  delayedSwitchOn(unsigned int delay)
  : delay(delay)
  {}
  delayedSwitchOn& operator++() { if (counter < delay) ++counter; return *this; }
  void reset() { counter = 0; }
  bool on()    { return counter == delay; }

private:
  const unsigned int delay;
  unsigned int counter = 0;
};

class Chip_card {
public:
  Chip_card();

  bool readCard (      nfcTagObject &nfcTag);
  bool writeCard(const nfcTagObject &nfcTag);
  void sleepCard();
  void initCard ();
  void stopCard ();
  void stopCrypto1();
  bool newCardPresent();
  bool cardRemoved();

private:
  MFRC522             &mfrc522;

  delayedSwitchOn     cardRemovedSwitch;
};




#endif /* SRC_CHIP_CARD_HPP_ */
