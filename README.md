# Simulation-Tests

Eine Sammlung an drei kleinen Programmen, die ich geschrieben habe, um die Simulation von Mehrkörperdynamik kennenzulernen. Es folgen einige Demo-Videos. Falls Sie das Programm selber ausführen wollen, benötigen sie die Raylib-Bibliothek als Abhängigkeit.

## Pendel

![Pendel-Demo](videos/pendel-demo.mp4)

Code: `pendulum.cpp`

## Kugeln im Raum

![20 Kugeln](videos/baelle-20-demo.mp4)

20 vollelastische Kugeln mit zufälliger Position, geschwindigkeit, Masse und Farbe werden in einen Raum "geworfen". Reibungsverluste an zwischen den Kugeln untereinander und an der Wand werden vereinfacht berücksichtigt. Außerdem gilt die Annahme, dass die Kugeln nicht rotieren.

![100 Kugeln](videos/baelle-100-demo.mp4)

Bei 100 Kugeln kommt das Programm nicht mehr mit - die Kugeln zittern umher und schieben sich mit der Zeit ineinander. Dies liegt daran, dass die Teilfunktion des Programms, der den Abstand zwischen den Bällen direkt nach einer Kollision regelt, einfach zu simpel geschrieben ist.

Code: `many_many_balls.cpp`

## Einfacher Wurf

![Wurf-Demo](videos/wurf-verlauf.mp4)

Diese simple Simulation einer im Raum geworfenen Kugel berücksichtigt die Schwerkraft und - in stark vereinfachter Form, es wird z.B. ein Konstanter cw-Wert angenommen - den Luftwiderstand. Ein tieferer Rotton in der Farbe der Kugel bedeutet eine höhere Widerstandskraft. Hier wird ebenso Rotation der Kugel ausgeschlossen.

Vor dem Beginn der Simulation wird der Benutzer gefragt, ob er den Verlauf der Flugkurve anzeigen lassen will. Dies visualisiert die Flugkurve des Balls und den Einfluss des Luftwiderstands deutlich besser.

Code: `ball_im_raum.cpp`