![](https://github.com/AlexanderWillner/tonuino/workflows/TonUINO%20Compiler%20Test/badge.svg)

# Änderungen in diesem Repository

* Unterstützung für GitHub Actions, um jede eingespielte Änderung zu testen (siehe Badge oben).
* Integration von PlatformIO inklusive initiale Unterstützung von Unit Tests.
* Entfernung einiger [Compiler-Warnungen](https://github.com/xfjx/TonUINO/pull/54).
* Kleinen Fehler bei Erstinbetriebnahme [gefixt](http://discourse.voss.earth/t/version-2-1dev-lautstaerke-durch-langes-druecken-der-tasten-geht-nicht/4523/23).
* Verzögerung für Lautstärkeregelung [hinzugefügt](http://discourse.voss.earth/t/lautstaerke-kleinschrittiger-erhoehen/3022/5).
* Make-Datei hinzugefügt:
```
$ make

TonUINO Makefile
================

Configured to use MCU "arduino:avr:nano:cpu=atmega328old" attached to port "/dev/cu.usbserial-1410".

Available commands:
- help    : get support from the community
- install : installation of required binaries (arduino-cli)
- init    : initialize environment (arduino-cli)
- compile : create binary
- upload  : store binary on board EEPROM
- find    : get information about the pluged-in board
- test    : run some basic tests on the code
- clean   : delete temporary files
```

# TonUINO
Die DIY Musikbox (nicht nur) für Kinder


# Change Log

## Version 2.1 (xx.xx.xxxx) noch WIP
- Partymodus hat nun eine Queue -> jedes Lied kommt nur genau 1x vorkommt
- Neue Wiedergabe-Modi "Spezialmodus Von-Bis" - Hörspiel, Album und Party -> erlaubt z.B. verschiedene Alben in einem Ordner zu haben und je mit einer Karte zu verknüpfen
- Admin-Menü
- Maximale, Minimale und Initiale Lautstärke
- Karten werden nun über das Admin-Menü neu konfiguriert
- die Funktion der Lautstärketasten (lauter/leiser oder vor/zurück) kann im Adminmenü vertauscht werden
- Shortcuts können konfiguriert werden!
- Support für 5 Knöpfe hinzugefügt
- Reset der Einstellungen ins Adminmenü verschoben
- Modikationskarten (Sleeptimer, Tastensperre, Stopptanz, KiTa-Modus)
- Admin-Menü kann abgesichert werden

## Version 2.01 (01.11.2018)
- kleiner Fix um die Probleme beim Anlernen von Karten zu reduzieren

## Version 2.0 (26.08.2018)

- Lautstärke wird nun über einen langen Tastendruck geändert
- bei kurzem Tastendruck wird der nächste / vorherige Track abgespielt (je nach Wiedergabemodus nicht verfügbar)
- Während der Wiedergabe wird bei langem Tastendruck auf Play/Pause die Nummer des aktuellen Tracks angesagt
- Neuer Wiedergabemodus: **Einzelmodus**
  Eine Karte kann mit einer einzelnen Datei aus einem Ordner verknüpft werden. Dadurch sind theoretisch 25000 verschiedene Karten für je eine Datei möglich
- Neuer Wiedergabemodus: **Hörbuch-Modus**
  Funktioniert genau wie der Album-Modus. Zusätzlich wir der Fortschritt im EEPROM des Arduinos gespeichert und beim nächsten mal wird bei der jeweils letzten Datei neu gestartet. Leider kann nur der Track, nicht die Stelle im Track gespeichert werden
- Um mehr als 100 Karten zu unterstützen wird die Konfiguration der Karten nicht mehr im EEPROM gespeichert sondern direkt auf den Karten - die Karte muss daher beim Anlernen aufgelegt bleiben!
- Durch einen langen Druck auf Play/Pause kann **eine Karte neu konfiguriert** werden
- In den Auswahldialogen kann durch langen Druck auf die Lautstärketasten jeweils um 10 Ordner oder Dateien vor und zurück gesprungen werden
- Reset des MP3 Moduls beim Start entfernt - war nicht nötig und hat "Krach" gemacht
