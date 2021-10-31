# For **Modus mean** use "-b MEAN https://github.com/AndreasLMeg/allsky.git"

# Allsky Camera - Erweiterung AndreasLMeg

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

### Neuinstallation
* D/A/CH: Installation wie im [readme](https://github.com/AndreasLMeg/allsky/blob/master/README.md) beschrieben (als Quelle verwende aber -b MEAN https://github.com/AndreasLMeg/allsky.git) 
* EN: Installation described in [Readme](https://github.com/AndreasLMeg/allsky/blob/master/README.md) should work (but use -b MEAN https://github.com/AndreasLMeg/allsky.git)

### bestehende Installation anpassen
Das Gelingen kann ich leider nicht garantieren, aber so könnte es funktionieren
1. Allsky stoppen
2. SSH Verbindung mit Raspberry erstellen (Putty)
3. Sicherungskopie anlegen
```shell
cp allsky/capture_RPiHQ allsky/capture_RPiHQ_ori 
cp allsky/keogram allsky/keogram_ori 
```
4. neue Software von Github holen 
```shell
git clone -b MEAN --recursive https://github.com/AndreasLMeg/allsky.git allsky_li
```
5. neue Programme erstellen
```shell
cd allsky_li
sudo make capture_RPiHQ
sudo make keogram
```
6. neue Programme in allsky Verzeichnis kopieren
```shell
cp capture_RPiHQ ~/allsky/capture_RPiHQ 
cp keogram ~/allsky/keogram 
```
7. Konfiguration wie unten beschrieben anpassen
8. Wichtig: allsky.sh und endOfNight.sh - dort müssen die entsprechenden Zeilen unbedingt eingefügt werden
9. Allsky starten
10. Viel Glück    

Hier mein Updatevorgang als Beweis ;-)
```shell
pi@allsky:~ $ cp allsky/capture_RPiHQ allsky/capture_RPiHQ_ori

pi@allsky:~ $ cp allsky/keogram allsky/keogram_ori

pi@allsky:~ $ git clone -b MEAN --recursive https://github.com/AndreasLMeg/allsky.git allsky_li
Klone nach 'allsky_li' ...
remote: Enumerating objects: 1990, done.
remote: Counting objects: 100% (427/427), done.
remote: Compressing objects: 100% (212/212), done.
remote: Total 1990 (delta 266), reused 320 (delta 198), pack-reused 1563
Empfange Objekte: 100% (1990/1990), 57.27 MiB | 2.19 MiB/s, Fertig.
Löse Unterschiede auf: 100% (1171/1171), Fertig.

pi@allsky:~ $ cd allsky_li

pi@allsky:~/allsky_li $ sudo make capture_RPiHQ
g++    -c -o mode_RPiHQ_mean.o mode_RPiHQ_mean.cpp
arm-linux-gnueabihf-g++  capture_RPiHQ.cpp mode_RPiHQ_mean.cpp -o capture_RPiHQ -Wall -Wno-psabi -g -D_LIN -D_DEBUG   -Llib/armv7 -I./include -lpthread  -DGLIBC_20 -I/usr/include/opencv -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_datasets -lopencv_dpm -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_line_descriptor -lopencv_optflow -lopencv_video -lopencv_plot -lopencv_reg -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_rgbd -lopencv_viz -lopencv_surface_matching -lopencv_text -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_xobjdetect -lopencv_objdetect -lopencv_ml -lopencv_xphoto -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core -lASICamera2 -I/usr/include/libusb-1.0 -lusb-1.0

pi@allsky:~/allsky_li $ sudo make keogram
arm-linux-gnueabihf-g++  keogram.cpp -o keogram -Wall -Wno-psabi -g -D_LIN -D_DEBUG   -Llib/armv7 -I./include -lpthread  -DGLIBC_20 -I/usr/include/opencv -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_datasets -lopencv_dpm -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_line_descriptor -lopencv_optflow -lopencv_video -lopencv_plot -lopencv_reg -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_rgbd -lopencv_viz -lopencv_surface_matching -lopencv_text -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_xobjdetect -lopencv_objdetect -lopencv_ml -lopencv_xphoto -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core

pi@allsky:~/allsky_li $ cp capture_RPiHQ ~/allsky/capture_RPiHQ

pi@allsky:~/allsky_li $ cp keogram ~/allsky/keogram
```


## Konfiguration (via GUI = HTML Konfiguration)

* D/A/CH:
Hier ein kurzer Überblick der neuen Möglichkeiten wenn der Modus aktiviert wurde:
### Camera settings
| Setting     | Default     | Additional Info |
| ----------- | ----------- | ----------------|
| exposure | 60000 | Die maximale Belichtungszeit [ms]. Kein Unterschied zwischen Tag und Nacht. Bei kürzeren Belichtungszeiten wird eine entsprechende Pause zwischen den Bildern eingelegt. Bei meinem Fisheye werden die Sterne bei 60s noch als Punkte abgebildet. |
| gain | 15 | Die maximal verwendete Verstärkung. [1..16] - Je nach Objektiv oder Filter (mit oder ohne IR Filter) sollte dieser Wert so eingestellt werden, dass bei einer dunklen Nacht die Milchstraße noch nicht im Rauschen untergeht.
| autogain | No | No: kein Autogain, Yes: Wähle 1 um die automatische Regelung zu ermöglichen.|

### Editor - [config.sh](https://github.com/AndreasLMeg/allsky/blob/master/config.sh.repo) 
In dieser Datei müssen ein paar zusätzliche Zeilen eingetragen bzw. angepasst werden. 

Für die Keogram Anpassungen:  [config.sh](https://github.com/AndreasLMeg/allsky/blob/master/config.sh.repo#L29)
```shell
# Set to true to generate a keogram at the end of the night (image summary of the night)
KEOGRAM=true

# keogram additional parameter
KEOGRAM_ADD_PARAMS="-fontsize 2.0 -fontline 2 -fontcolor 255 255 255"
```

Und am Ende noch die Anpassungen "Mode Mean"  [config.sh](https://github.com/AndreasLMeg/allsky/blob/master/config.sh.repo#L84)
```shell
# Additional parameter - mode mean
#ADD_PARAMS="$ADD_PARAMS -mode 1 -mean-value 0.50 -mean-threshold 0.02 -mean-shuttersteps 6 -mean-fastforward 4.0 -mean-longplay 0 -mean-historySize 3 -mean-maskHorizon 0 -mean-info 1"
ADD_PARAMS="$ADD_PARAMS -mode 1 -mean-info 1"
```

Hier ist eine Liste der möglichen Zusatzparameter
| Configuration     | Default     | Additional Info |
| ----------- | ----------- | ----------------|
| ADD_PARAMS  | "$ADD_PARAMS -mode 1 -mean-info 1" | Das ist der empfohlene Wert, es können aber auch weitere Parameter angegeben werden                |
|             | "$ADD_PARAMS" | ein eventuell schon vorher definierter Parameter wird übernommen
|             | "-mode 1" | [0]: Mode Mean ist deaktiviert, 1: Mode Mean ist aktiviert
|             | "-mean-info 1" | [0]: keine Zusatzinfos, 1: Werte für shutter und gain werden eingeblendet, 2: weitere Infos werden angezeigt 
|             | "-mean-value 0.5" | Mittelwert des Bildes, es können werte von 0.0 bis 1.0 eingegeben werden
|             | "-mean-threshold 0.2" | Ab dieser Abweichung vom Mittelwert startet die Regelung
|             | "-mean-shuttersteps 6" | Wie von der Fotografie bekannt gibt es bestimmte Werte für die Belichtungszeit (0.5s, 1s, 2s, 4s, 8s,...) Die Einstellung ermöglicht auch Zwischenwerte, d.h. die Regelung kann präzisier erfolgen.
|             | "-mean-fastforward 4.0" | Dieser Wert bestimmt wie stark auf Regelabweichungen reagiert wird. (4.0 wurde empirisch ermittelt) Kleinere Werte führen zu einem langsamen Regelverhalten, größere Werte können aber schnell zu unerwünschten Schwingungen führen
|             | "-mean-longplay 0" | [0]: Bei kürzeren Belichtungszeiten wird eine Pause eingelegt. 1: Keine Pause, daher in der Dämmerung mehr Bilder und das Video wird länger.
|             | "-mean-historySize 3" | Ab dieser Abweichung vom Mittelwert startet die Regelung
|             | "-mean-maskHorizon 0" | 0: Keine Schwärzung 1: Es wird ein Bild (Maske) als Vorlage zur Verfügung gestellt. Mit einem Grafikprogramm deiner Wahl können unerwünschte Bereich geschwärzt werden. Eine genauere Beschreibung folgt...
|             | "-mean-quickstart 10" | Um sich schneller Einzuregeln, werden 10 Bilder ohne Pause gemacht.
| KEOGRAM_ADD_PARAMS  | "-fontsize 2.0 -fontline 2 -fontcolor 255 255 255" | Das ist der empfohlene Wert, es können aber auch weitere Parameter angegeben werden                |
|             | "-fontsize 2.0" | Zeichensatzgröße
|             | "-fontline 2" | 
|             | "-fontcolor 255 255 255" | Textfarbe
|             | "-finishline 809" | Gibt an welche Spalte des Ursprungsbilds ins Keogramm übernommen wird (https://github.com/thomasjacquin/allsky/issues/387)
|             | "-addRow 1" | 0: nur eine Spalte pro Bild - das ergibt aber meist sehr schmale Keogramme 1: Spalte wird mehrfach verwendet, um ein breiteres Bild zu erhalten. 2: Die Nachbarspalten werden verwendet. 

### Editor - [allsky.sh](https://github.com/AndreasLMeg/allsky/blob/master/allsky.sh#L80) 
Diese Datei sollte die Zeile "ARGUMENTS="$ARGUMENTS $ADD_PARAMS"" enthalten. Bei einem Update einer bestehenden Installation müssen die Zeilen manuell angepaßt werden.

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



### Editor - [endOfNight.sh](https://github.com/AndreasLMeg/allsky/blob/master/scripts/endOfNight.sh#L37)
Diese Datei sollte die Zeile "../keogram $ALLSKY_HOME/images/$LAST_NIGHT/ $EXTENSION $ALLSKY_HOME/images/$LAST_NIGHT/keogram/keogram-$LAST_NIGHT.$EXTENSION $KEOGRAM_ADD_PARAMS" enthalten. Bei einem Update einer bestehenden Installation müssen die Zeieln manuell angepaßt werden.

```shell
...
# Generate keogram from collected images
if [[ $KEOGRAM == "true" ]]; then
        echo -e "Generating Keogram\n"
        mkdir -p $ALLSKY_HOME/images/$LAST_NIGHT/keogram/
        ../keogram $ALLSKY_HOME/images/$LAST_NIGHT/ $EXTENSION $ALLSKY_HOME/images/$LAST_NIGHT/keogram/keogram-$LAST_NIGHT.$EXTENSION $KEOGRAM_ADD_PARAMS
...
```

Hinweis: Diese .sh Dateien sind ausführbare Skripte. Das bedeutet, dass es bei Änderungen auch zu Fehlfunktionen kommen kann. Bei Problemen bitte immer die das Logging kontrollieren !
```shell
cat /var/log/allsky.log
```


## Aktivierung

Nachdem config.sh und camera settings geändert wurden, MUSS Allsky neu gestartet werden (System - Stop Allsky - Start Allsky).


## Exif information

Alle gespeicherten Bilder enthalten nun eine zusätzliche EXIF Information "Artist=li_1000000_1".  -> shuttertime: 1000000µs, gain=1  

## Donation

If you found this project useful, here's a link to send me a cup of coffee :)

[![](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=44GLWL8SYVVXQ&source=url)
