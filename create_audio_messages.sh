#!/bin/sh
mkdir advert
mkdir mp3
for i in {1..255};
do
  j=$(printf "%04d" $i)
  say -v Anna -o $j.aiff $i
  ffmpeg -y -i $j.aiff -acodec libmp3lame -ab 128k -ac 1 advert/$j.mp3 -acodec libmp3lame -ab 128k -ac 1 mp3/$j.mp3
  rm $j.aiff
done
awk -F'|' '{system("say -v Anna -o outfile.aiff " $2 " && ffmpeg -y -i outfile.aiff -acodec libmp3lame -ab 128k -ac 1 " $1 " && rm outfile.aiff")}' < audio_messages.txt
