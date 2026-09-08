# Simulation-Tests

Eine Sammlung an drei kleinen Programmen, die ich geschrieben habe, um die Simulation von Mehrkörperdynamik kennenzulernen. Es folgen einige Demo-Videos. Falls Sie das Programm selber ausführen wollen, benötigen Sie die Raylib-Bibliothek als Abhängigkeit.

## Doppelpendel

https://github.com/user-attachments/assets/094a404c-bbf0-4152-a249-40b4d44851ad

Zwei Punktmassen sind miteinander verbunden, eine fest gelagert. Das System wird ausgelenkt und losgelassen.
Die Simulation ist auf Kraft basiert, nachträglich wird die Geometrie korrigiert, um die Zwangsbedingung der Stäbe durchzusetzen. Gelöst wird mit einem expliziten Euler-Integrationsverfahren. Ein Problem mit dieser Methode ist, dass sie ohne weitere Behandlung viel Energie in das System einführt und bei höheren Auslenkungswinkeln dazu führt, dass das System immer schneller, bis ins undendliche schwingt, aber auch bei kleineren Auslenkungen zu unrealistischen Schwingungsvorgängen führt.
Dieser Effekt kann mit einer iterativen Anwendung der zwei Zwangsbedingungen und einer sehr hohen Zahl an Substeps (Simulationsschritten, die zwischen zwei Frames geschehen), stark vermindert werden. Jedoch führt das zu einer relativ hohen CPU-Auslastung für so eine einfache Simulation. Die Anwendung ist also stark begrenzt.

Code: `pendulum.cpp`

## Kugeln im Raum

https://github.com/user-attachments/assets/4a2bbb91-7d7f-4a91-bc39-ca05440ddda4

20 vollelastische Kugeln mit zufälliger Position, Geschwindigkeit, Masse und Farbe werden in einen Raum "geworfen". Reibungsverluste zwischen den Kugeln untereinander und an der Wand werden vereinfacht berücksichtigt. Außerdem gilt die Annahme, dass die Kugeln nicht rotieren.

https://github.com/user-attachments/assets/84f08bec-f6a7-4b5b-b65c-9fc22f01117d

Bei 100 Kugeln kommt das Programm nicht mehr mit - die Kugeln zittern umher und schieben sich mit der Zeit ineinander. Dies liegt daran, dass die Teilfunktion des Programms, die den Abstand zwischen den Bällen direkt nach einer Kollision regelt, einfach zu simpel geschrieben ist.

Code: `many_many_balls.cpp`

## Einfacher Wurf

https://github.com/user-attachments/assets/c8faa81a-ac4a-4d98-bda5-3724e8da444b

Diese simple Simulation einer im Raum geworfenen Kugel berücksichtigt die Schwerkraft und - in stark vereinfachter Form (es wird z. B. ein konstanter cw-Wert angenommen) - den Luftwiderstand. Ein tieferer Rotton in der Farbe der Kugel bedeutet eine höhere Widerstandskraft. Hier wird ebenso die Rotation der Kugel ausgeschlossen.

Vor dem Beginn der Simulation wird der Benutzer gefragt, ob er den Verlauf der Flugkurve anzeigen lassen will. Dies visualisiert die Flugkurve des Balls und den Einfluss des Luftwiderstands deutlich besser.

Code: `ball_im_raum.cpp`
