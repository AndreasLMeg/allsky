# Allsky Camera - Erweiterung AndreasLMeg



This is the source code for the Wireless Allsky Camera project described [on Instructables](http://www.instructables.com/id/Wireless-All-Sky-Camera/).


![](http://www.thomasjacquin.com/allsky-portal/screenshots/camera-header-photo.jpg)

## Warum wurde dieser Fork gemacht:

* Angeregt von Daniel Nimmervoll (https://youtu.be/jBPuhz8ju6A) und aus Interesse an Astronomie, Programmieren und Experimentieren wollte ich mir das ständige Anpassen der Einstellungen ersparen und habe mir ein paar Verbesserungen und Erweiterungen einfallen lassen.
* Ich werde das Ursprungsprojekt von Thomas Jacquin (https://github.com/thomasjacquin/allsky) zwar weiter beobachten, aber nicht alle Anpassungen übernehmen. (Da fehlt mir wieder die Zeit dazu)
* Hinweis: Leider ist die Regelung noch nicht perfekt, da wird es sicher noch Verbesserungen geben. Aber es ist schwierig die Belichtungssituation der nächsten Minute vorherzusehen.

### Anpassungen
1. Es wird versucht ein optimal belichtetes Bild zu erzeugen. Dazu wird der Mittelwert des Bildes berechnet und die Belichtungszeit und Verstärkung entsprechend angepasst.
2. Die Belichtungszeit und die Verstärkung ist in den EXIF Daten abgelegt (Autoren)
3. Für die Berechnung des Mittelwerts wird nur die Mittes des Bilds herangezogen. Dadurch verschwinden die Einflüsse von Beleuchtungen, Autos, usw.
4. Die Regelung erfolgt mit einer empirisch ermittelten Formel. Meine Versuche mit PID Reglern scheiterte.
5. Es können bei Bedarf Bildbereiche geschwärzt werden (Schlafzimmerfenser vom Nachbarn,...)
6. Zusatzparameter in config.sh.
7. Es werden nun historische Werte für die Regelung herangezogen 
8. Es wird nun eine Progrose berechnet, dieser Wert beeinflußt die neu berechnete Belichtungszeit
9. Keogram: Zusatzparameter in config.sh.
10. Keogram: Wenn die Kamera nicht optimal nach Norden ausgerichtet ist, bzw. sich störende Elemente in der Bildmitte befinden, kann nun die Spalte des Bildes gewählt werden. Siehe auch https://github.com/thomasjacquin/allsky/issues/387
11. Keogram: Es können nun Zusatzspalten eingefügt werden. Bei großer Auflösung und kurzen Nächten werden die Keogramme sonst sehr schmal.
12. Keogram: Es wird nun auch der Datumswechsel angezeigt.
13. Keogram: Die Schrift ist nun deutlicher zu lesen (Schwarzer Text ist hinterlegt)
 

## Installation

* D/A/CH: Installation wie im [readme](https://github.com/AndreasLMeg/allsky/blob/master/README.md) beschrieben (als Quelle verwende aber https://github.com/AndreasLMeg/allsky.git) 
* EN: Installation described in [Readme](https://github.com/AndreasLMeg/allsky/blob/master/README.md) should work (but use https://github.com/AndreasLMeg/allsky.git)

## Configuration (via GUI = HTML Konfiguration)

* D/A/CH:
Hier ein kurzer Überblick der neuen Möglichkeiten wenn der Modus aktiviert wurde:
### Camera settings
| Setting     | Default     | Additional Info |
| ----------- | ----------- | ----------------|
| exposure | 60000 | Die maximale Belichtungszeit [ms]. Kein Unterschied zwischen Tag und Nacht. Bei kürzeren Belichtungszeiten wird eine entsprechende Pause zwischen den Bildern eingelegt. |
| gain | 15 | Die maximale Verstärkung. [1..16]|
| autogain | No | No: kein Autogain, Yes: Wähle 1 um die automatische Regelung zu ermöglichen.|

### Editor - config.sh

| Configuration     | Default     | Additional Info |
| ----------- | ----------- | ----------------|
| ADD_PARAMS  | "$ADD_PARAMS -mode 1 -mean-info 1" | Das ist der empfohlene Wert, es können aber auch weitere Parameter angegeben werden                |
|             | "$ADD_PARAMS" | ein eventuell schon vorher definierter Parameter wird übernommen
|             | "-mode 1" | [0]: Mode Mean ist deaktiviert, 1: Mode Mean ist aktiviert
|             | "-mean-info 1" | [0]: keine Zusatzinfos, 1: Werte für shutter und gain werden eingeblendet, 2: weitere Infos werden angezeigt 
|             | "-mean-value 0.5" | Mittelwert des Bildes, es können werte von 0.0 bis 1.0 eingegeben werden
|             | "-mean-threshold 0.2" | Ab dieser Abweichung vom Mittelwert startet die Regelung
|             | "-mean-shuttersteps 6" | Wie von der Fotografie bekannt gibt es bestimmte Werte für die Belichtungszeit (0.5s, 1s, 2s, 4s, 8s,...) Die Einstellung ermöglicht auch Zwischenwerte, d.h. die Regelung kann präzisier erfolgen.
|             | "-mean-fastforward 4.0" | Dieser Wert bestimmt wie stark auf regelabweichungen reagiert wird. (4.0 wurde empirisch ermittelt) Kleinere Werte führen zu einem langsamen Regelverhalten, größere Werte können aber schnell zu unerwünschten Schwingungen führen
|             | "-mean-longplay 0" | [0]: Bei kürzeren Belichtungszeiten wird eine Pause eingelegt. 1: Keine Pause, daher in der Dämmerung mehr Bilder.
|             | "-mean-historySize 3" | Ab dieser Abweichung vom Mittelwert startet die Regelung
|             | "-mean-maskHorizon 0" | 0: Keine Schwärzung 1: Es wird ein Bild (Maske) als Vorlage zur Verfügung gestellt. Mit einem Grafikprogramm deiner Wahl können unerwünschte Bereich geschwärzt werden. Eine genauere Beschreibung folgt...
|             | "-mean-quickstart 10" | Um sich schneller Einzuregeln, werden 10 Bilder ohne Pause gemacht.
| KEOGRAM_ADD_PARAMS  | "-fontsize 2.0 -fontline 2 -fontcolor 255 255 255" | Das ist der empfohlene Wert, es können aber auch weitere Parameter angegeben werden                |
|             | "-fontsize 2.0" | Zeichensatzgröße
|             | "-fontline 2" | 
|             | "-fontcolor 255 255 255" | Textfarbe
|             | "-finishline 809" | Gibt an welche Spalte des Ursprungsbilds ins Keogramm übernommen wird (https://github.com/thomasjacquin/allsky/issues/387)
|             | "-addRow 1" | 0: nur eine Spalte pro Bild - das ergibt aber meist sehr schmale Keogramme 1: Spalte wird mehrfach verwendet, um ein breiteres Bild zu erhalten. 2: Die Nachbarspalten werden verwendet. 

### Editor - allsky.sh
Diese Datei sollte die Zeile "ARGUMENTS="$ARGUMENTS $ADD_PARAMS"" enthalten:

```shell
...
# When using a desktop environment (Remote Desktop, VNC, HDMI output, etc), a preview of the capture can be displayed in a separate window
# The preview mode does not work if allsky.sh is started as a service or if the debian distribution has no desktop environment.
if [[ $1 == "preview" ]] ; then
	ARGUMENTS="$ARGUMENTS -preview 1"
fi
ARGUMENTS="$ARGUMENTS -daytime $DAYTIME"

# Additional parameters
ARGUMENTS="$ARGUMENTS $ADD_PARAMS"

echo "$ARGUMENTS">>log.txt
...
```

### Editor - endOfNight.sh
Diese Datei sollte die Zeile "../keogram $ALLSKY_HOME/images/$LAST_NIGHT/ $EXTENSION $ALLSKY_HOME/images/$LAST_NIGHT/keogram/keogram-$LAST_NIGHT.$EXTENSION $KEOGRAM_ADD_PARAMS" enthalten:

```shell
...
# Generate keogram from collected images
if [[ $KEOGRAM == "true" ]]; then
        echo -e "Generating Keogram\n"
        mkdir -p $ALLSKY_HOME/images/$LAST_NIGHT/keogram/
        ../keogram $ALLSKY_HOME/images/$LAST_NIGHT/ $EXTENSION $ALLSKY_HOME/images/$LAST_NIGHT/keogram/keogram-$LAST_NIGHT.$EXTENSION $KEOGRAM_ADD_PARAMS
...
```


## Usage

### Activation

Change parameter in config.sh and camera settings.


### Exif information

All images should have exif information "Artist=li_1000000_1".  -> shuttertime: 1000000µs, gain=1  

## Donation

If you found this project useful, here's a link to send me a cup of coffee :)

[![](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=44GLWL8SYVVXQ&source=url)
