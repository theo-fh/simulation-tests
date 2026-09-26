#include <raylib.h>
#include <string>
#include <string.h>
#include "pendulum-objects.h"
#include "pendulum-functions.h"
#include <iostream>

enum operation{
    Excite,
    Suppress,
    Stop
};

int main(){

    //Lager bestimmen
    constexpr int win_dimensions[] = {600, 400};
    constexpr float motor_init_x = (float) win_dimensions[0]/2.0f;
    constexpr float motor_init_y = 60.0f;

    //erste Punktmasse initiieren
    constexpr float acceleration_g = 150.0f;

    //Masse nach Länge und Auslenkung einführen
    constexpr float length_init = 200.0f;
    constexpr float angle_init = 0.0f * PI / 180.0f;
    const float mass1_init_x = motor_init_x + sinf(angle_init) * length_init;
    const float mass1_init_y = motor_init_y + cosf(angle_init) * length_init;

    //Längen der Stäbe und Ausschlagswinkel holen, false als erster Parameter,
    //um den Nutzer nicht nach Werten zu fragenm sondern Standardwerte einzusetzen
    //getInitValues(false, bearing_x, bearing_y, mass1_init_x, mass1_init_y, motor_init_x, motor_init_y);


    
    //Masse und Motor erstellen
    PointMass mass1("m1", 2.5f, mass1_init_x, mass1_init_y, acceleration_g);
    constexpr float max_speed_motor = 100.0f;
    Motor motor1(motor_init_x, motor_init_y, max_speed_motor);

    //Lager und Massen miteinander verbinden
    RodLink rod1(&mass1, &motor1);

    //Fenster initiieren
    InitWindow(win_dimensions[0], win_dimensions[1], "Doppelpendel");
    constexpr int targetFPS = 60;
    constexpr float speed_factor = 2.5f; //lässt die simulation im zeitraffer ablaufen
    SetTargetFPS(targetFPS);
    constexpr float frame_dt = 1.0f/targetFPS;

    //Schriftart und Bild des Festlagers Laden
    Font notoserif = LoadFontEx("fonts/NotoSerif-VariableFont_wdth,wght.ttf", 20, NULL, 0);
    Texture2D bearingImg = LoadTexture("images/bearing.png");
    constexpr  int bearingImgWidth = 48;
    constexpr  int bearingImgCircleHeight = 41;

    //Vektor mit Positionsverlauf des Pendels initiieren
    std::vector<Vector2> trajectory;
    constexpr int vel_tang_length = 3;

    //Motor starten für Bewegung
    motor1.vel_x = 0.0f;
    motor1.reference_pos_x = motor_init_x;

    //Simulation starten
    constexpr int sub_steps = 100;
    constexpr float dt = (frame_dt * speed_factor) / (float)sub_steps;

    bool equilibrium_detected = false;
    operation curr_operation = Excite;
    while(!WindowShouldClose()){

        //Koordinaten in Substeps erneuern
        for(int step = 0; step < sub_steps; step++){
            
            if(mass1.pos_y < motor1.pos_y && equilibrium_detected) curr_operation = Suppress;
            else if (curr_operation == Suppress && mass1.pos_y > motor1.pos_y) curr_operation = Stop;

            if(curr_operation == Excite){
                constexpr float position_change_from_excite = 50.0f; //wirklich keine Ahnung wie ich das nennen soll
                equilibrium_detected = motor1.excite(&mass1, equilibrium_detected, position_change_from_excite); //Motor soll Pendel aufschwingen
            }
            else if(curr_operation == Suppress){
                constexpr float position_change_factor = 3.0f; //gibt an, wie stark der motor dem pendel nachfolgt (stabilisiert Pendel)
                constexpr float speed_change_factor = 0.5f; //gibt an, wie viel der motor der masse voraus fährt (hält Motor an einem Ort)
                equilibrium_detected = motor1.suppress(&mass1, equilibrium_detected, position_change_factor, speed_change_factor);
            }
            else if(curr_operation == Stop) motor1.reference_pos_x = win_dimensions[0]/2.0f;

            constexpr float speed_control_factor = 2.0f;
            motor1.controlSpeed(speed_control_factor); //Motor zur Führungsgröße hinbewegen

            mass1.updateAcc();

            mass1.updateVel(dt);

            mass1.updatePos(dt);

            motor1.updatePos(dt);
            motor1.backAndForth(win_dimensions[0], 100);
            
            //Zwangsbedingung der zwei Stäbe iterativ durchsetzen, vermindert Überschwingen
            rod1.correctPosition();
            rod1.correctVelocity();

        }


        //Zeichnen
        BeginDrawing();
        ClearBackground(WHITE);

        //Verlauf der unteren Masse
        traceMass(&mass1, trajectory);

        //Stäbe und Massen
        rod1.draw();
        mass1.drawAsCircle(notoserif);
        //std::cout << mass1.forces_x[0] << "\n";

        //Lager
        DrawTexture(bearingImg, (int) motor1.pos_x - bearingImgWidth/2, (int) motor1.pos_y - bearingImgCircleHeight, WHITE);
        EndDrawing();
    }

    return 0;
}