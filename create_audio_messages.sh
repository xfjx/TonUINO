#!/bin/sh
mkdir mp3
awk -F'|' '{system("say -v Anna -o outfile.aiff " $2 " && ffmpeg -y -i outfile.aiff -acodec libmp3lame -ab 128k -ac 1 " $1 " && rm outfile.aiff")}' < audio_messages.txt
