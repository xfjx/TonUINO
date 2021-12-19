#ifndef SRC_TONUINO_HPP_
#define SRC_TONUINO_HPP_

#include "settings.hpp"
#include "buttons.hpp"
#include "mp3.hpp"
#include "modifier.hpp"

class Tonuino {
public:
  Tonuino() {}

  void setup         ();
  void loop          ();

  void playFolder      ();
  void playShortCut    (uint8_t shortCut);
  void playCurrentTrack() { if (knownCard) mp3.playFolderTrack(myFolder->folder, getCurrentTrack()); }

  void     nextTrack();
  void previousTrack();

  void resetActiveModifier() { activeModifier = &noneModifier; }

  void setStandbyTimer();

  void setCard  (const nfcTagObject   &newCard  ) { myCard = newCard; setFolder(&myCard.nfcFolderSettings); }
  void setFolder(const folderSettings *newFolder) { myFolder = newFolder; }

private:

  uint8_t getCurrentTrack() const;

  void handleButtons ();
  void handleChipCard();
  void writeCard(const nfcTagObject &nfcTag);

  void checkStandbyAtMillis();
  void disableStandbyTimer ();

  void volumeUpButton  ();
  void volumeDownButton();
  void nextButton      ();
  void previousButton  ();

  bool setupFolder(folderSettings& theFolder);

  void setupCard  ();
  bool specialCard(const nfcTagObject &nfcTag);


  bool adminMenuAllowed();
  void adminMenu       ();

  void voiceMenuPlayOption( uint8_t   returnValue
                          , mp3Tracks messageOffset
                          , bool      preview
                          , int       previewFromFolder);
  uint8_t voiceMenu( int       numberOfOptions
                   , mp3Tracks startMessage
                   , mp3Tracks messageOffset
                   , bool      preview = false
                   , int       previewFromFolder = 0
                   , int       defaultValue = 0
                   , bool      exitWithLongPress = false);
  void createModifierCard  ();
  void createCardsForFolder();
  void shuffleQueue        ();

  static const size_t  maxTracksInFolder = 255;
  typedef array<uint8_t, maxTracksInFolder> queue_t;

  Settings             settings            {};
  Mp3                  mp3                 {settings};
  Buttons              buttons             {settings};
  Chip_card            chip_card           {mp3, buttons};


  Modifier             noneModifier        {*this, mp3, settings};
  SleepTimer           sleepTimer          {*this, mp3, settings};
  FreezeDance          freezeDance         {*this, mp3, settings};
  Locked               locked              {*this, mp3, settings};
  ToddlerMode          toddlerMode         {*this, mp3, settings};
  KindergardenMode     kindergardenMode    {*this, mp3, settings};
  RepeatSingleModifier repeatSingleModifier{*this, mp3, settings};
  //FeedbackModifier     feedbackModifier    {*this, mp3, settings};

  Modifier     *activeModifier    = &noneModifier;

  uint16_t      numTracksInFolder = 0;
  uint16_t      currentTrack      = 0;
  uint16_t      firstTrack        = 0;
  queue_t       queue{};

  unsigned long standbyAtMillis   = 0;

  bool          knownCard         = false;

  nfcTagObject          myCard;
  const folderSettings *myFolder  = &myCard.nfcFolderSettings;

};

extern Tonuino tonuino;



#endif /* SRC_TONUINO_HPP_ */
