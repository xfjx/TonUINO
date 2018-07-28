#!/bin/bash

for i in {01..99}; 
do 
    i=$(printf "%02d" $i)
    say -v anna $i -o $i.aiff
    sox $i.aiff $i.wav pitch 800
    lame -b 128 $i.wav 00$i.mp3
done

say -v Anna "Oh, eine neue Karte! Verwende die Lautstärke Knöpfe um einen Order für die Karte auszuwählen. Drücke die Pause Taste um die Auswahl zu speichern." -o 0100.aiff
sox 0100.aiff 0100.wav pitch 800
lame -b 128 0100.wav 0100.mp3

say -v Anna "OK, ich habe die Karte mit dem Ordner verknüpft. Wähle nun mit den Lautstärke Knöpfen den Wiedergabemodus aus." -o 0101.aiff
sox 0101.aiff 0101.wav pitch 800
lame -b 128 0101.wav 0101.mp3

say -v Anna "Hörspielmodus: Ein zufälliges Lied aus dem Ordner wiedergeben" -o 0102.aiff
sox 0102.aiff 0102.wav pitch 800
lame -b 128 0102.wav 0102.mp3

say -v Anna "Albummodus: Den kompletten Ordner wiedergeben" -o 0103.aiff
sox 0103.aiff 0103.wav pitch 800
lame -b 128 0103.wav 0103.mp3

say -v Anna "Party Modus: Ordner zufällig wiedergeben." -o 0104.aiff
sox 0104.aiff 0104.wav pitch 800
lame -b 128 0104.wav 0104.mp3

say -v Anna "Einzel Modus: Eine bestimmte Datei im Ordner wiedergeben." -o 0105.aiff
sox 0105.aiff 0105.wav pitch 800
lame -b 128 0105.wav 0105.mp3

say -v Anna "OK. Ich habe die Karte konfiguriert." -o 0110.aiff
sox 0110.aiff 0110.wav pitch 800
lame -b 128 0110.wav 0110.mp3

rm *.aiff
rm *.wav
