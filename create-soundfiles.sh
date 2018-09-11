#!/bin/bash

for i in {1..250};
do
    j=$(printf "%04d" $i)
    say -v anna $i -o $j.aiff
    sox $j.aiff $j.wav pitch 800
    lame -b 128 $j.wav $j.mp3
done

say -v Anna "Oh, eine neue Karte! Verwende die Lautstärke Tasten um einen Order für die Karte auszuwählen. Drücke die Pause Taste um die Auswahl zu speichern." -o 0300.aiff
sox 0300.aiff 0300.wav pitch 800
lame -b 128 0300.wav 0300_new_tag.mp3

say -v Anna "OK, ich habe die Karte mit dem Ordner verknüpft. Wähle nun mit den Lautstärke Tasten den Wiedergabemodus aus." -o 0310.aiff
sox 0310.aiff 0310.wav pitch 800
lame -b 128 0310.wav 0310.mp3

say -v Anna "Hörspielmodus: Eine zufällige Datei aus dem Ordner wiedergeben" -o 0311.aiff
sox 0311.aiff 0311.wav pitch 800
lame -b 128 0311.wav 0311_mode_random_episode.mp3

say -v Anna "Albummodus: Den kompletten Ordner wiedergeben" -o 0312.aiff
sox 0312.aiff 0312.wav pitch 800
lame -b 128 0312.wav 0312_mode_album.mp3

say -v Anna "Party Modus: Ordner zufällig wiedergeben." -o 0313.aiff
sox 0313.aiff 0313.wav pitch 800
lame -b 128 0313.wav 0313_mode_party.mp3

say -v Anna "Einzel Modus: Eine bestimmte Datei im Ordner wiedergeben." -o 0314.aiff
sox 0314.aiff 0314.wav pitch 800
lame -b 128 0314.wav 0314_mode_sigle_track.mp3

say -v Anna "Hörbuch Modus: Einen Ordner wiedergeben und den Fortschritt speichern." -o 0315.aiff
sox 0315.aiff 0315.wav pitch 800
lame -b 128 0315.wav 0315_node_audio_book.mp3

say -v Anna "OK, wähle nun bitte die Datei mit den Lautstärke Tasten aus. " -o 0320.aiff
sox 0320.aiff 0320.wav pitch 800
lame -b 128 0320.wav 0320_select_file.mp3

say -v Anna "Admin Funktionen." -o 0316.aiff
sox 0316.aiff 0316.wav pitch 800
lame -b 128 0316.wav 0316_admin.mp3

say -v Anna "Bitte lege die zu löschende Karte auf! Zum Abbrechen einfach eine der Lautstärke Tasten drücken!" -o 0800.aiff
sox 0800.aiff 0800.wav pitch 800
lame -b 128 0800.wav 0800_reset_tag.mp3

say -v Anna "OK, du kannst den Tag nun wieder neu konfigurieren." -o 0801.aiff
sox 0801.aiff 0801.wav pitch 800
lame -b 128 0801.wav 0801_reset_tag_ok.mp3

say -v Anna "OK, ich habe den Vorgang abgebrochen." -o 0802.aiff
sox 0802.aiff 0802.wav pitch 800
lame -b 128 0802.wav 0802_reset_aborted.mp3

say -v Anna "Reset wurde durchgeführt!" -o 0999.aiff
sox 0999.aiff 0999.wav pitch 800
lame -b 128 0999.wav 0999_reset_ok.mp3

say -v Anna "Soll ich vor einer Datei jeweils die Nummer ansagen? Du kannst jederzeit durch einen langen Druck auf die Pause Taste die aktuelle Nummer abfragen." -o 0330.aiff
sox 0330.aiff 0330.wav pitch 800
lame -b 128 0330.wav 0330.mp3

say -v Anna "Nein, Nummer nicht ansagen." -o 0331.aiff
sox 0331.aiff 0331.wav pitch 800
lame -b 128 0331.wav 0331.mp3

say -v Anna "Ja, Nummer ansagen." -o 0332.aiff
sox 0332.aiff 0332.wav pitch 800
lame -b 128 0332.wav 0332.mp3

say -v Anna "OK. Ich habe die Karte konfiguriert." -o 0400.aiff
sox 0400.aiff 0400.wav pitch 800
lame -b 128 0400.wav 0400_ok.mp3

say -v Anna "Oh weh! Das hat leider nicht geklappt!." -o 0401.aiff
sox 0401.aiff 0401.wav pitch 800
lame -b 128 0401.wav 0401_error.mp3


rm *.aiff
rm *.wav
