#ifndef LOGGING_H
#define LOGGING_H

#include <SoftwareSerial.h>

/*
 * Global serial device.
 */
SoftwareSerial mySoftwareSerial(2, 3); // RX, TX

/**
 * Global serial device.
 */
void log_buffer(byte * buffer, byte bufferSize);

/**
 * Logging macros.
 */
#if defined(DEBUG_VERBOSE)
#  define DEBUG_VERBOSE_PRINTLN(x) Serial.println(x)
#  define DEBUG_VERBOSE_PRINT(x)   Serial.print(x)
#  define DEBUG_PRINTLN(x)         Serial.println(x)
#  define DEBUG_PRINT(x)           Serial.print(x)
#  define DEBUG_PRINTI(x, b)       Serial.print(x, b)
#  define DEBUG_PRINTB(b, l)       log_buffer(b, l)
#elif defined(DEBUG)
#  define DEBUG_VERBOSE_PRINTLN(x)
#  define DEBUG_VERBOSE_PRINT(x)
#  define DEBUG_PRINTLN(x)      Serial.println(x)
#  define DEBUG_PRINT(x)        Serial.print(x)
#  define DEBUG_PRINTI(x, b)    Serial.print(x, b)
#  define DEBUG_PRINTB(b, l)    log_buffer(b, l)
#else
#  define DEBUG_VERBOSE_PRINTLN(x)
#  define DEBUG_VERBOSE_PRINT(x)
#  define DEBUG_PRINTLN(x)
#  define DEBUG_PRINT(x)
#  define DEBUG_PRINTI(x, b)
#  define DEBUG_PRINTB(b, l)
#endif

#endif
