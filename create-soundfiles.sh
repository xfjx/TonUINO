#!/bin/bash

for i in {1..255};
do
    j=$(printf "%04d" $i)
    say -v anna "$i" -o $j.aiff
    sox $j.aiff $j.wav pitch 800
    lame -b 128 $j.wav $j.mp3
done

say -v Anna "Oh, eine neue Karte! Verwende die Lautstärke Tasten um einen Ordner für die Karte auszuwählen. Drücke die Pause Taste um die Auswahl zu speichern." -o 0300.aiff
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
lame -b 128 0314.wav 0314_mode_single_track.mp3

say -v Anna "Hörbuch Modus: Einen Ordner wiedergeben und den Fortschritt speichern." -o 0315.aiff
sox 0315.aiff 0315.wav pitch 800
lame -b 128 0315.wav 0315_mode_audio_book.mp3

say -v Anna "Admin Funktionen." -o 0316.aiff
sox 0316.aiff 0316.wav pitch 800
lame -b 128 0316.wav 0316_admin.mp3

say -v Anna "Spezialmodus Von-Bis, Hörspiel: Eine zufällige Datei zwischen der Start und Enddatei wiedergeben." -o 0317.aiff
sox 0317.aiff 0317.wav pitch 800
lame -b 128 0317.wav 0317_special_random.mp3

say -v Anna "Spezialmodus Von-Bis, Album: Alle Dateien zwischen der Start und Enddatei wiedergeben." -o 0318.aiff
sox 0318.aiff 0318.wav pitch 800
lame -b 128 0318.wav 0318_special_album.mp3

say -v Anna "Spezialmodus Von-Bis, Party: Alle Dateien zwischen der Start und Enddatei zufällig wiedergeben." -o 0319.aiff
sox 0319.aiff 0319.wav pitch 800
lame -b 128 0319.wav 0319_special_party.mp3

say -v Anna "OK, wähle nun bitte die Datei mit den Lautstärke Tasten aus. " -o 0320.aiff
sox 0320.aiff 0320.wav pitch 800
lame -b 128 0320.wav 0320_select_file.mp3

say -v Anna "OK, wähle nun bitte die Startdatei mit den Lautstärke Tasten aus. " -o 0321.aiff
sox 0321.aiff 0321.wav pitch 800
lame -b 128 0321.wav 0321_select_first_file.mp3

say -v Anna "Wähle nun bitte die Enddatei mit den Lautstärke Tasten aus. " -o 0322.aiff
sox 0322.aiff 0322.wav pitch 800
lame -b 128 0322.wav 0322_select_last_file.mp3

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

say -v Anna "Bitte lege nun die Karte auf! Zum Abbrechen einfach eine der Lautstärke Tasten drücken!" -o 0800.aiff
sox 0800.aiff 0800.wav pitch 800
lame -b 128 0800.wav 0800_reset_tag.mp3

say -v Anna "OK, du kannst den Tag nun wieder neu konfigurieren." -o 0801.aiff
sox 0801.aiff 0801.wav pitch 800
lame -b 128 0801.wav 0801_reset_tag_ok.mp3

say -v Anna "OK, ich habe den Vorgang abgebrochen." -o 0802.aiff
sox 0802.aiff 0802.wav pitch 800
lame -b 128 0802.wav 0802_reset_aborted.mp3

say -v Anna "Willkommen im Admin Menü. Bitte wähle eine Funktion mit den Lautstärke Tasten aus und bestätige sie mit der Pause Taste!" -o 0900.aiff
sox 0900.aiff 0900.wav pitch 800
lame -b 128 0900.wav 0900_admin.mp3

say -v Anna "Eine Karte neu konfigurieren." -o 0901.aiff
sox 0901.aiff 0901.wav pitch 800
lame -b 128 0901.wav 0901_card_reset.mp3

say -v Anna "Maximale Lautstärke festlegen." -o 0902.aiff
sox 0902.aiff 0902.wav pitch 800
lame -b 128 0902.wav 0902_max_volume.mp3

say -v Anna "Minimale Lautstärke festlegen." -o 0903.aiff
sox 0903.aiff 0903.wav pitch 800
lame -b 128 0903.wav 0903_min_volume.mp3

say -v Anna "Lautstärke beim Start festlegen." -o 0904.aiff
sox 0904.aiff 0904.wav pitch 800
lame -b 128 0904.wav 0904_init_volume.mp3

say -v Anna "EQ konfigurieren." -o 0905.aiff
sox 0905.aiff 0905.wav pitch 800
lame -b 128 0905.wav 0905_eq.mp3

say -v Anna "Eine Masterkarte erstellen." -o 0906.aiff
sox 0906.aiff 0906.wav pitch 800
lame -b 128 0906.wav 0906_mastercard.mp3

say -v Anna "Tasten mit einem Shortcut konfigurieren." -o 0907.aiff
sox 0907.aiff 0907.wav pitch 800
lame -b 128 0907.wav 0907_shortcut.mp3

say -v Anna "Den Standbytimer konfigurieren." -o 0908.aiff
sox 0908.aiff 0908.wav pitch 800
lame -b 128 0908.wav 0908_sleeptimer.mp3

say -v Anna "Einzelkarten für einen Ordner erstellen." -o 0909.aiff
sox 0909.aiff 0909.wav pitch 800
lame -b 128 0909.wav 0909_batch_cards.mp3

say -v Anna "Funktion der Lautstärke Tasten umdrehen." -o 0910.aiff
sox 0910.aiff 0910.wav pitch 800
lame -b 128 0910.wav 0910_batch_cards.mp3

say -v Anna "Bitte wähle eine Einstellung für den EQ mit den Lautstärke Tasten aus und bestätige sie mit der Pause Taste." -o 0920.aiff
sox 0920.aiff 0920.wav pitch 800
lame -b 128 0920.wav 0920_eq_intro.mp3

say -v Anna "Normal" -o 0921.aiff
sox 0921.aiff 0921.wav pitch 800
lame -b 128 0921.wav 0921_normal.mp3

say -v Anna "Pop" -o 0922.aiff
sox 0922.aiff 0922.wav pitch 800
lame -b 128 0922.wav 0922_pop.mp3

say -v Anna "Rock" -o 0923.aiff
sox 0923.aiff 0923.wav pitch 800
lame -b 128 0923.wav 0923_rock.mp3

say -v Anna "Jazz" -o 0924.aiff
sox 0924.aiff 0924.wav pitch 800
lame -b 128 0924.wav 0924_jazz.mp3

say -v Anna "Classic" -o 0925.aiff
sox 0925.aiff 0925.wav pitch 800
lame -b 128 0925.wav 0925_classic.mp3

say -v Anna "Bass" -o 0926.aiff
sox 0926.aiff 0926.wav pitch 800
lame -b 128 0926.wav 0926_bass.mp3

say -v Anna "Maximale Lautstärke wählen und mit der Pause Taste bestätigen." -o 0930.aiff
sox 0930.aiff 0930.wav pitch 800
lame -b 128 0930.wav 0930_max_volume.mp3

say -v Anna "Minimale Lautstärke wählen und mit der Pause Taste bestätigen." -o 0931.aiff
sox 0931.aiff 0931.wav pitch 800
lame -b 128 0931.wav 0931_min_volume.mp3

say -v Anna "Lautstärke beim Start wählen und mit der Pause Taste bestätigen." -o 0932.aiff
sox 0932.aiff 0932.wav pitch 800
lame -b 128 0932.wav 0932_init_volume.mp3

say -v Anna "Möchtest du die Funktion der Lautstärke Tasten umdrehen? Du musst dann die Tasten lange drücken um ein Lied vor oder zurückzugehen." -o 0933.aiff
sox 0933.aiff 0933.wav pitch 800
lame -b 128 0933.wav 0933_switch_volume.mp3

say -v Anna "Nein." -o 0934.aiff
sox 0934.aiff 0934.wav pitch 800
lame -b 128 0934.wav 0934_no.mp3

say -v Anna "Ja." -o 0935.aiff
sox 0935.aiff 0935.wav pitch 800
lame -b 128 0935.wav 0935_yes.mp3


say -v Anna "Reset wurde durchgeführt!" -o 0999.aiff
sox 0999.aiff 0999.wav pitch 800
lame -b 128 0999.wav 0999_reset_ok.mp3

rm *.aiff
rm *.wav
