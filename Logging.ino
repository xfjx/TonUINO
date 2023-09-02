#include "Logging.h"

/*
 * Helper routine to dump a byte array as hex values to serial.
*/
void log_buffer(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
	  Serial.print(buffer[i] < 0x10 ? " 0" : " ");
	  Serial.print(buffer[i], HEX);
  }
}
