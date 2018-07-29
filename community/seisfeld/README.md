TonUINO Community Edition
=========================

Hier ist die erste Version der TonUINO Firmware aus der Community. Da ich das Glück hatte, beim _Betatest_ dabei sein zu dürfen, habe ich für mich selber mit der Zeit immer weitere Funktionen nachgerüstet. Vielleicht sind diese ja für den ein oder anderen ebenfalls interessant:

## Funktionen

- **Erweiterung:** Die Verknüpfung der einzelnen Ordner zu den NFC Tags wird auf den Tags selbst und nicht mehr im EEPROM des Arduinos gespeichert. Damit gibt es im Prinzip kein Limit mehr für die Anzahl der unterstützten Tags.
- **Erweiterung:** Die serielle Konsole gibt zu Debug Zwecken allerlei nützlicher Daten aus, so dass man beim basteln zu jeder Zeit weiss was gerade passiert.
- **Erweiterung:** Man kann das anlernen von NFC Tags abbrechen (beide Laustärketasten 2sec gerückt halten).
- **Erweiterung:** Wenn TonUINO gerade keinen Titel spielt, kommt man mit "beide Laustärketasten 2sec gerückt halten“ in den NFC Tag löschen Modus, zum abbrechen des selbigen wiederum "beide Laustärketasten 2sec gerückt halten“. Somit kann man zu jeder Zeit auch einzelne NFC Tags wieder löschen um sie danach neu verwenden zu können.
- **Erweiterung:** Wenn TonUINO gerade einen Titel spielt und sich im Album Modus befindet, kann man mit den beiden Laustärketasten (jeweils 2sec gerückt halten) zum nächsten oder vorherigen Titel springen. Im Party Modus ebenfalls, allerdings dann nur zum nächsten Titel.
- **Erweiterung:** Man kann im Sketch die Maximallautstärke zum Schutz der Kinderohren (und der Nerven der Eltern ;-)) zwischnen 0 und 30 festlegen.
- **Erweiterung:** Wenn man einen passenden IR Empfänger nachrüstet, kann man TonUINO auch fernsteuern. Momentan sind die codes für zwei verschiedene Apple Fernbedienungen hinterlegt, das lässt sich aber für andere Fernbedienungen anpassen.
- **Erweiterung:** Wenn man eine LED nachrüstet, werden mit dieser LED ein paar nützliche Informationen angezeigt. Man sieht dann z.B. ob TonUINO gerade einen Titel spielt (LED pulsiert langsam), sich im NFC Tag löschen oder anlernen Modus befindet (LED blinkt alle 500ms), oder einfach nur idle ist (LED leuchtet dauerhaft).
- **Bugfix:** Man kann während man ein NFC Tag anlernt nicht mehr bestätigen ohne vorher wirklich sowohl Ordner als auch Wiedergabemodus ausgewählt zu haben.

## Dokumentation

Der Sketch ist relativ gut dokumentiert und am Anfang ist auch nochmal alles zusammen gefasst (allerdings auf Englisch). Dort finden sich dann auch nochmal eine generelle Übersicht der Funktionen und Informationen wie man die Pins definiert usw., weil das von Aufbau zu Aufbau verschieden sein kann. Ebenfalls ist dort erklärt wo der optionale IR Empfänger und die LED anzuschliessen sind etc. etc.

## Audio Meldungen

Ihr könnt die benötigten Audiomeldunden natürlich selbst einsprechen (siehe `audio_messages.txt`). Wem das zu aufwendig ist, kann das beigelegte shell script `create_audio_messages.sh` verwenden, welches alle MP3s in einem Rutsch erzeugt. Dazu wird sowohl das `say` Kommando (von MacOS) als auch `ffmpeg` benötigt. Der erzeugte Ordner `mp3` ist so wie er ist auf die SD Karte zu kopieren. Bitte achtet darauf, dass wirklich keine anderen Dateien (Stichwort MacOS...) auf der Karte landen, da das MP3 Modul da sehr pingelig ist.

## Lizenz

GPL v3. Siehe `LICENSE` Datei.
