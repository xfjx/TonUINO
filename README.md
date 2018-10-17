Alternative TonUINO Firmware
============================

Hier ist eine alternative Version der TonUINO Firmware. Da ich das Glück hatte, beim _Betatest_ von TonUINO dabei sein zu dürfen, habe ich für mich selber mit der Zeit immer weitere Funktionen nachgerüstet. Vielleicht sind diese ja für den ein oder anderen ebenfalls interessant. Die u.a. Änderungen beziehen sich auf die TonUINO Firmware in der Version 1.0. Einige der Funktionen sind mittlerweile auch in der originalen TonUINO Firmware zu finden.

## Funktionen

- **Erweiterung:** Die Verknüpfung der einzelnen Ordner zu den NFC Tags wird auf den Tags selbst und nicht mehr im EEPROM des Arduinos gespeichert. Damit gibt es im Prinzip kein Limit mehr für die Anzahl der unterstützten Tags.
- **Erweiterung:** Die serielle Konsole gibt zu Debug Zwecken allerlei nützlicher Daten aus, so dass man beim basteln zu jeder Zeit weiss was gerade passiert.
- **Erweiterung:** Man kann das anlernen von NFC Tags abbrechen (beide Laustärketasten 2sec gerückt halten).
- **Erweiterung:** Wenn TonUINO gerade keinen Titel spielt, kommt man mit "beide Laustärketasten 2sec gerückt halten“ in den NFC Tag löschen Modus, zum abbrechen des selbigen wiederum "beide Laustärketasten 2sec gerückt halten“. Somit kann man zu jeder Zeit auch einzelne NFC Tags wieder löschen um sie danach neu verwenden zu können.
- **Erweiterung:** Wenn TonUINO gerade einen Titel spielt und sich im Album Modus befindet, kann man mit den beiden Laustärketasten (jeweils 2sec gerückt halten) zum nächsten oder vorherigen Titel springen. Im Party Modus ebenfalls, allerdings dann nur zum nächsten Titel.
- **Erweiterung:** Man kann im Sketch die Maximallautstärke zum Schutz der Kinderohren (und der Nerven der Eltern ;-)) zwischnen 0 und 30 festlegen.
- **Erweiterung:** Wenn man einen passenden IR Empfänger nachrüstet, kann man TonUINO auch fernsteuern. Momentan sind die codes für zwei verschiedene Apple Fernbedienungen hinterlegt, das lässt sich aber für andere Fernbedienungen anpassen.
- **Erweiterung:** Wenn man eine LED nachrüstet, werden mit dieser LED ein paar nützliche Informationen angezeigt. Man sieht dann z.B. ob TonUINO gerade einen Titel spielt (LED leuchtet dauerhaft), sich im NFC Tag löschen oder anlernen Modus befindet (LED blinkt alle 500ms), oder einfach nur idle ist (LED pulsiert langsam).
- **Erweiterung:** Man kann mit der IR Fernbedienung die Buttons und den NFC Leser von TonUINO sperren.
- **Erweiterung:** Lieblingsfolge. Es kann nun auch ein einzelner Titel aus einem Ordner mit einem NFC Tag verknüpft werden. Kinder lieben bekanntlich Wiederholungen. ;-) _Diesen Modus habe ich aus der 2.0 der originalen TonUINO Firmware übernommen._
- **Erweiterung:** Hörbuchmodus. Spielt wie der Albummodus den gesammten Ordner, merkt sich aber den letzten Titel. Wenn man während der Wiedergabe die beiden Laustärketasten für 2sec gedrückt hällt, startet die Wiedergabe wieder von vorne. _Diesen Modus habe ich aus der 2.0 der originalen TonUINO Firmware übernommen._
- **Bugfix:** Man kann während man ein NFC Tag anlernt nicht mehr bestätigen ohne vorher wirklich sowohl Ordner als auch Wiedergabemodus ausgewählt zu haben.
- **Bugfix:** Umgang mit bestimmten Versionen des DFPlayer Mini Moduls verbessert.

## Dokumentation

Der Sketch ist relativ gut dokumentiert und am Anfang ist auch nochmal alles zusammen gefasst (allerdings auf Englisch). Dort finden sich dann auch nochmal eine generelle Übersicht der Funktionen und Informationen wie man die Pins definiert usw., weil das von Aufbau zu Aufbau verschieden sein kann. Ebenfalls ist dort erklärt wo der optionale IR Empfänger und die LED anzuschliessen sind etc. etc.

## Audio Meldungen

Ihr könnt die benötigten Audiomeldunden natürlich selbst einsprechen (siehe `audio_messages.txt`). Wem das zu aufwendig ist, kann das beigelegte shell script `create_audio_messages.sh` verwenden, welches alle MP3s in einem Rutsch erzeugt. Dazu wird sowohl das `say` Kommando (von MacOS) als auch `ffmpeg` benötigt. Die erzeugten Ordner `mp3` und `advert` sind so wie sie sind auf die SD Karte zu kopieren. Bitte achtet darauf, dass wirklich keine anderen Dateien (Stichwort .DS_Store usw. unter MacOS...) auf der Karte landen, da das MP3 Modul da sehr pingelig ist. Details dazu gibt es auch nochmal auf der Homepage des Projekts.

## Lizenz

GPL v3. Siehe [LICENSE](https://github.com/seisfeld/TonUINO/blob/master/LICENSE) Datei.
