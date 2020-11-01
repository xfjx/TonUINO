# How-To:
# Arduino-Makefile auschecken
# git clone git@github.com:sudar/Arduino-Makefile.git@github
#
# Makefile erstellen, folgende Variablen setzen
# ARDUINO_DIR – Arduino-IDE  Ordner
# ARDMK_DIR – Order in dem Arduino-Makefile ausgecheckt wurde
# entweder direkt im Makefile oder alse Umgebungsvariable (mittles export z.B.)
#
# bauen und hochladen:
# make upload
#
# Debuggen:
# make monitor
#
# Beispiel:

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
