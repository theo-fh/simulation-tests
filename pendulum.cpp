#include <raylib.h>
#include <string>
#include <string.h>
#include "pendulum-objects.h"
#include "pendulum-functions.h"

int main(){

    //Lager bestimmen
    int win_dimensions[] = {600, 400};
    float bearing_x = (float) win_dimensions[0]/2.0f;
    float bearing_y = 60.0f;

    //erste Punktmasse initiieren
    float acceleration_g = 150.0f;

    //Masse nach Länge und Auslenkung einführen
    float mass1_init_x;
    float mass1_init_y;
    float mass2_init_x;
    float mass2_init_y;

    //Längen der Stäbe und Ausschlagswinkel holen, false als erster Parameter,
    //um den Nutzer nicht nach Werten zu fragenm sondern Standardwerte einzusetzen
    getInitValues(false, bearing_x, bearing_y, mass1_init_x, mass1_init_y, mass2_init_x, mass2_init_y);
    
    //Massen erstellen
    PointMass mass1("m1", 2.5f, mass1_init_x, mass1_init_y, acceleration_g);
    PointMass mass2("m2", 3.0f, mass2_init_x, mass2_init_y, acceleration_g);

    //Lager und Massen miteinander verbinden
    RodFix rod1(bearing_x, bearing_y, &mass1);
    RodConnect rod2(&mass1, &mass2);

    //Fenster initiieren
    InitWindow(win_dimensions[0], win_dimensions[1], "Doppelpendel");
    int targetFPS = 60;
    float speed_factor = 2.5f; //lässt die simulation im zeitraffer ablaufen
    SetTargetFPS(targetFPS);

    //Schriftart und Bild des Festlagers Laden
    Font notoserif = LoadFontEx("fonts/NotoSerif-VariableFont_wdth,wght.ttf", 20, NULL, 0);
    Texture2D bearingImg = LoadTexture("images/bearing.png");
    int bearingImgWidth = 48;
    int bearingImgCircleHeight = 41;

    //Vektor mit Positionsverlauf des Pendels initiieren
    std::vector<Vector2> trajectory;

    //Simulation starten
    while(!WindowShouldClose()){
        float frame_dt = GetFrameTime();

        //Explizite Euler-Integration fügt dem System schnell Energie hinzu, deshalb sehr hohe Frequenz nötig
        int sub_steps = 800;

        float dt = (frame_dt * speed_factor) / (float)sub_steps;

        //Koordinaten in Substeps erneuern
        for(int step = 0; step < sub_steps; step++){
            rod1.update();
            rod2.update();

            mass1.updateAcc();
            mass2.updateAcc();

            mass1.updateVel(dt);
            mass2.updateVel(dt);

            mass1.updatePos(dt);
            mass2.updatePos(dt);
            
            //Zwangsbedingung der zwei Stäbe iterativ durchsetzen, vermindert Überschwingen
            for (int iteration = 0; iteration < 20; iteration++) {
                rod2.correctPosition();
                rod1.correctPosition();
            }
            rod2.correctVelocity();
            rod1.correctVelocity();

        }


        //Zeichnen
        BeginDrawing();
        ClearBackground(WHITE);

        //Verlauf der unteren Masse
        traceMass(&mass2, trajectory);

        //Stäbe und Massen
        rod1.draw();
        rod2.draw();
        mass1.drawAsCircle(notoserif);
        mass2.drawAsCircle(notoserif);

        //Lager
        DrawTexture(bearingImg, (int) bearing_x - bearingImgWidth/2, (int) bearing_y - bearingImgCircleHeight, WHITE);
        EndDrawing();
    }

    return 0;
}