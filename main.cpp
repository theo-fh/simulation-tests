#include <raylib.h>
#include <string>
#include <string.h>
#include "pendulum-objects.h"
#include "pendulum-functions.h"
#include <iostream>

int main(){

    //Lager bestimmen
    constexpr int win_dimensions[] = {600, 400};
    constexpr float motor_init_x = (float) win_dimensions[0]/2.0f;
    constexpr float motor_init_y = (float) win_dimensions[1]/2.0f;

    //erste Punktmasse initiieren
    constexpr float acceleration_g = 150.0f;

    //Masse nach Länge und Auslenkung einführen
    constexpr float length_init = 150.0f;
    constexpr float angle_init = 0.0f * PI / 180.0f;
    const float mass1_init_x = motor_init_x + sinf(angle_init) * length_init;
    const float mass1_init_y = motor_init_y + cosf(angle_init) * length_init;

    //Längen der Stäbe und Ausschlagswinkel holen, false als erster Parameter,
    //um den Nutzer nicht nach Werten zu fragenm sondern Standardwerte einzusetzen
    //getInitValues(false, bearing_x, bearing_y, mass1_init_x, mass1_init_y, motor_init_x, motor_init_y);

    //Masse und Motor erstellen
    constexpr float mass_given = 2.5f;
    PointMass mass1(":)", mass_given, mass1_init_x, mass1_init_y, acceleration_g);
    constexpr float max_speed_motor = 200.0f;
    constexpr float max_acc_motor = 400.0f;
    Motor motor1(motor_init_x, motor_init_y, max_speed_motor, max_acc_motor);

    //Lager und Massen miteinander verbinden
    RodLink rod1(&mass1, &motor1);

    //Fenster initiieren
    InitWindow(win_dimensions[0], win_dimensions[1], "Doppelpendel");
    constexpr int targetFPS = 60;
    constexpr float speed_factor = 2.5f; //lässt die simulation im zeitraffer ablaufen
    SetTargetFPS(targetFPS);
    constexpr float frame_dt = 1.0f/targetFPS;

    //Schriftart und Bild des Festlagers Laden
    constexpr int loaded_fontsize = 20;
    Font notoserif = LoadFontEx("fonts/NotoSerif-VariableFont_wdth,wght.ttf", loaded_fontsize, NULL, 0);
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
    bool last_equilibrium = false;
    operation curr_operation = Excite;
    while(!WindowShouldClose()){

        float velSq = mass1.vel_x * mass1.vel_x + mass1.vel_y * mass1.vel_y;

        //TODO: diesen wert optimal bestimmen
        constexpr float surplus_speed_for_suppress = 30000.0f;
        float velSq_needed_for_flip = 4.0f * (length_init + mass1.pos_y - motor1.pos_y) * acceleration_g;
        //std::cout << velSq << "     " << velSq_needed_for_flip << "\n";
    
        constexpr float max_border_dist = 30.0f;
        if(motor1.pos_x < max_border_dist | motor1.pos_x > win_dimensions[0] - max_border_dist) curr_operation = Center;

        constexpr float center_tolerance = 50.0f;

        if(curr_operation == Center && abs(motor1.pos_x - win_dimensions[0] / 2) > center_tolerance) {}

        else if(velSq > velSq_needed_for_flip + surplus_speed_for_suppress) curr_operation = Suppress; 

        else if(mass1.pos_y < motor1.pos_y && equilibrium_detected) curr_operation = Stabilize;

        else if (mass1.pos_y > motor1.pos_y ) curr_operation = Excite;

        if(IsKeyDown(KEY_SPACE)) curr_operation = Stop;

        constexpr float push_force = 80.0f;
        float push_factor;
        if(IsKeyDown(KEY_LEFT)) push_factor = mass1.push('L', push_force);
        else if (IsKeyDown(KEY_RIGHT)) push_factor = mass1.push('R', push_force);
        else push_factor = mass1.push('-', 0.0f);

        if(curr_operation == Excite){
            constexpr float position_change_from_excite = 50.0f; //wirklich keine Ahnung wie ich das nennen soll
            motor1.excite(&mass1, equilibrium_detected, last_equilibrium, position_change_from_excite); //Motor soll Pendel aufschwingen
        }
        else if(curr_operation == Stabilize){
                //gibt an, wie stark der motor dem pendel nachfolgt (stabilisiert Pendel)
            constexpr float speed_change_factor = 1.6f; //gibt an, wie viel der motor der masse voraus fährt (hält Motor an einem Ort)
            motor1.stabilize(&mass1, speed_change_factor);
        }

        else if (curr_operation == Suppress){
            constexpr float speed_change_factor = 2.0f;
            constexpr float suppress_interval = 20.0f;
            motor1.suppress(&mass1, suppress_interval, speed_change_factor, win_dimensions[0]);
        }
        else if(curr_operation == Stop) motor1.reference_pos_x = win_dimensions[0]/2.0f;

        else if(curr_operation == Center){
            constexpr float wiggleroom = 40.0f;
            motor1.center(&mass1, wiggleroom, win_dimensions[0]);
        };

        last_equilibrium = equilibrium_detected;
        //Verhältnis zwischen geschwindigkeit und Intervall, in dem Gleichgewicht erkannt wird
        constexpr float tolerance_factor = 5.0f;
        equilibrium_detected = detectEquilibrium(&mass1, &motor1, tolerance_factor);


        //Koordinaten in Substeps erneuern
        for(int step = 0; step < sub_steps; step++){

            rod1.update();
            
            //equilibrium_detected = detectEquilibrium(&mass1, &motor1);

            constexpr float speed_control_factor = 2.0f;
            motor1.controlSpeedP(speed_control_factor, dt); //Motor zur Führungsgröße hinbewegen

            mass1.updateAcc();

            mass1.updateVel(dt);

            mass1.updatePos(dt);

            motor1.updatePos(dt);
            //motor1.backAndForth(win_dimensions[0], 100);
            
            //Zwangsbedingung korrigieren
            rod1.correctPosition();
            rod1.correctVelocity();

        }


        //Zeichnen
        BeginDrawing();
        ClearBackground(WHITE);

        //Verlauf der Masse und Stellwert der Motor-Position
        traceMass(&mass1, trajectory);
        motor1.drawReference();
        printOperation(curr_operation, notoserif, loaded_fontsize);

        //Stäbe und Massen
        rod1.draw();
        mass1.drawAsCircle(notoserif);
        constexpr float draw_force_scale = 0.8f;
        if(push_factor != 0.0f) drawForcePush(&mass1, push_factor, 0.0f, push_force, draw_force_scale);
        //std::cout << mass1.forces_x[0] << "\n";

        //Lager
        DrawTexture(bearingImg, (int) motor1.pos_x - bearingImgWidth/2, (int) motor1.pos_y - bearingImgCircleHeight, WHITE);
        EndDrawing();
    }

    return 0;
}