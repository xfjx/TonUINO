#!/bin/bash
mkdir mp3

awk -F'|' '{system("say -v Anna -o outfile.aiff " $2 " && sox outfile.aiff outfile.wav pitch 800 && lame -b 128 outfile.wav " $1 " && rm outfile.aiff outfile.wav")}' < soundfiles.txt

for i in {1..255};
do
    j=$(printf "%04d" $i)
    say -v anna "$i" -o outfile.aiff
    sox outfile.aiff outfile.wav pitch 800
    lame -b 128 outfile.wav mp3/$j.mp3
    rm outfile.aiff outfile.wav
done

