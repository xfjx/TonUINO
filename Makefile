#
# TonUINO Makefile
#
# Author: Alexander Willner
# Date: 2020-01-03

# Config
## Arduino Nano (old bootloader)
MCU = arduino:avr:nano:cpu=atmega328old
BOARD = arduino:avr
SERIAL = /dev/cu.usbserial-1410
## Main
SKETCH = Tonuino
## Helper
OS = $(shell uname)
.PHONY: help

help:
	$(info TonUINO Makefile)
	$(info ================)
	$(info )
	$(info Configured to use MCU "$(MCU)" attached to port "$(SERIAL)".)
	$(info )
	$(info Available commands:)
	$(info - install : installation of required binaries (arduino-cli))
	$(info - init    : initialize environment)
	$(info - compile : create binary)
	$(info - upload  : store binary on board EEPROM)
	$(info - find    : get information about the pluged-in board)
	$(info - test    : run some basic tests on the code)
	$(info - clean   : delete temporary files)
	@true

install:
ifeq ($(OS),Darwin)
ifeq (, $(shell which brew))
	$(error "No brew in PATH, consider installing http://brew.sh")
else
	@brew install arduino-cli
endif
else
	@echo "todo: auto setup for OS"
endif

init:
	@arduino-cli config init
	@arduino-cli core update-index
	@arduino-cli core install $(BOARD)
	
compile:
	@mkdir -p $(SKETCH) ; echo "folder is only for backwards compatibility" > $(SKETCH)/DO-NOT-EDIT
	@if [ "$(SKETCH)/$(SKETCH).ino" -nt "./$(SKETCH).ino" ]; then echo "ERROR: do not edit files in TonUINO/!"; exit 1; fi;
	@cp -p ./$(SKETCH).ino $(SKETCH)/$(SKETCH).ino
	@arduino-cli compile --fqbn $(MCU) --warnings none $(SKETCH)

find:
	@arduino-cli board list

upload:
	@arduino-cli upload -p $(SERIAL) --fqbn $(MCU) --verify $(SKETCH)

test:
	@arduino-cli compile --fqbn $(MCU) --warnings more $(SKETCH)

clean:
	@rm -rf "$(SKETCH)"