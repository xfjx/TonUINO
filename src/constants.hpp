#ifndef SRC_CONSTANTS_HPP_
#define SRC_CONSTANTS_HPP_

#include <Arduino.h>

// ####### helper for level ############################

enum class level {
  inactive,
  active  ,
};
enum class levelType {
  activeHigh,
  activeLow,
};

inline int getLevel(levelType t, level l) { return (l == level::inactive) ? (t==levelType::activeHigh ? LOW  : HIGH)
                                                                          : (t==levelType::activeHigh ? HIGH : LOW ); }

// ####### buttons #####################################

// uncomment the below line to enable five button support
//#define FIVEBUTTONS

const uint32_t buttonLongPress      = 1000; // timeout for long press button in ms
const uint8_t  buttonPausePin       = A0;
const uint8_t  buttonUpPin          = A1;
const uint8_t  buttonDownPin        = A2;

#ifdef FIVEBUTTONS
const uint8_t  buttonFourPin        = A3;
const uint8_t  buttonFivePin        = A4;
#endif

const levelType buttonPinType       = levelType::activeLow;
const uint32_t  buttonDbTime        = 25; // Debounce time in milliseconds (default 25ms)

// ####### chip_card ###################################

const uint32_t cardCookie           = 322417479;
const byte     mfrc522_RSTPin       =  9;          // Configurable, see typical pin layout above
const byte     mfrc522_SSPin        = 10;          // Configurable, see typical pin layout above

// ####### mp3 #########################################

const uint8_t   dfPlayer_receivePin  = 2;
const uint8_t   dfPlayer_transmitPin = 3;
const uint8_t   dfPlayer_busyPin     = 4;
const levelType dfPlayer_busyPinType = levelType::activeHigh;


// ####### tonuino #####################################

const uint8_t   shutdownPin          = 7;
const levelType shutdownPinType      = levelType::activeHigh;
const uint8_t   openAnalogPin        = A7;
const unsigned long cycleTime        = 100;

#endif /* SRC_CONSTANTS_HPP_ */
