# inspired by https://github.com/SpenceKonde/ATTinyCore/blob/master/makefile.md

ARDUINO_DIR = ${HOME}/devel/ext/arduino-1.8.12

BASTEL_DIR = ${HOME}/devel/basteln
PROJECT_DIR = $(BASTEL_DIR)/TonUINO
ARDUINO_SKETCHBOOK = $(BASTEL_DIR)
ARDMK_DIR = $(BASTEL_DIR)/Arduino-Makefile

USER_LIB_PATH = $(PROJECT_DIR)/lib

BOARD_TAG = nano
BOARD_SUB = atmega328old

MONITOR_PORT = /dev/ttyUSB0

OBJDIR = $(PROJECT_DIR)/build
TARGET = TonUINO

ARDUINO_LIBS = DFPlayer JC_Button MFRC522 EEPROM SPI SoftwareSerial

include $(ARDMK_DIR)/Arduino.mk
