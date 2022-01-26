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

inline constexpr uint32_t buttonLongPress      = 1000; // timeout for long press button in ms
inline constexpr uint8_t  buttonPausePin       = A0;
inline constexpr uint8_t  buttonUpPin          = A1;
inline constexpr uint8_t  buttonDownPin        = A2;

#ifdef FIVEBUTTONS
inline constexpr uint8_t  buttonFourPin        = A3;
inline constexpr uint8_t  buttonFivePin        = A4;
#endif

inline constexpr levelType buttonPinType       = levelType::activeLow;
inline constexpr uint32_t  buttonDbTime        = 25; // Debounce time in milliseconds (default 25ms)

// ####### chip_card ###################################

inline constexpr uint32_t cardCookie           = 0x1337b347;
inline constexpr uint8_t  cardVersion          = 0x02;
inline constexpr byte     mfrc522_RSTPin       =  9;          // Configurable, see typical pin layout above
inline constexpr byte     mfrc522_SSPin        = 10;          // Configurable, see typical pin layout above

// ####### mp3 #########################################

inline constexpr uint8_t       dfPlayer_receivePin      = 2;
inline constexpr uint8_t       dfPlayer_transmitPin     = 3;
inline constexpr uint8_t       dfPlayer_busyPin         = 4;
inline constexpr levelType     dfPlayer_busyPinType     = levelType::activeHigh;
inline constexpr unsigned long dfPlayer_timeUntilStarts = 300;


// ####### tonuino #####################################

inline constexpr uint8_t   shutdownPin          = 7;
inline constexpr levelType shutdownPinType      = levelType::activeHigh;
inline constexpr uint8_t   openAnalogPin        = A7;
inline constexpr unsigned long cycleTime        = 50;

#endif /* SRC_CONSTANTS_HPP_ */
