#include <raylib.h>
#include <cmath>
#include <iostream>

int main() {

    char verlaufWahl[10];
    printf("Verlauf anzeigen (j/n)?");
    fflush(stdout);
    fgets(verlaufWahl,2,stdin);
    
    InitWindow(800, 800, "test");
    SetTargetFPS(60);

    double g = 9.81; // m/s²
    double x_speed_init = 40.0/3.6; // wert km/h mit faktor 3.6 in m/s umgewandelt
    double y_speed_init = -20.0/3.6;
    double dt = 1.0 / 60.0; // s - umkehrwert der framerate. wichtig für berechnung mit zeit.
    double scale = 40.0 / 1.0; // Maßstab. 1/1 (entspricht 1:1) heißt dass ein pixel einen Meter entspricht
    double totalSpeed = 0.0;
    Color ballColor = {0, 120, 0, 255};
    double ballRadius = 0.3;
    double masse = 3;
    double c_w = 0.2;
    double dichte_luft = 1.2; //kg/m³
    double flaeche_projiziert = 2 * 3.14 * pow(ballRadius,2);
    double stossfaktor = 0.6; // ist nicht wirklich der stoßfaktor aber die berechnung tu ich mir nicht an

    float x = 30 / scale;
    float y = 300.0 / scale;


    double x_speed = x_speed_init; //hat sich als unnötig erwiesen
    double y_speed = y_speed_init;

    // Loop
    while(WindowShouldClose() == false){

        // Widerstandskraft einbeziehen (Luftreibung)
        totalSpeed = sqrt(pow(x_speed,2) + pow(y_speed,2));
        
        double direction_x = x_speed / totalSpeed;
        double direction_y = y_speed / totalSpeed;

        double widerstandskraft = 0.5 * c_w * dichte_luft * flaeche_projiziert * pow(totalSpeed,2);
        double wind_deacc = widerstandskraft / masse;


        double colorScale = 20;
        if(widerstandskraft*colorScale<255){
            ballColor.r = widerstandskraft* colorScale;
        }
        else{
            ballColor.r = 255;
        }


        //std::cout << y_speed << " ";

        // Positionen updaten
        x += x_speed * dt;
        y += y_speed * dt;

        double x_acc = 0 - direction_x * wind_deacc;
        double y_acc = g - direction_y * wind_deacc;

        x_speed += x_acc * dt;
        y_speed += y_acc * dt;
        

        //Stöße gegen die Wand
        if( (x > (800)/scale - ballRadius && x_speed> 0) | (x < (0)/scale + ballRadius && x_speed < 0) ){
            x_speed = -stossfaktor * x_speed;
        }
        if( (y > (800)/scale - ballRadius && y_speed > 0) | (y < (0)/scale + ballRadius && y_speed < 0) ){
            y_speed = -stossfaktor * y_speed;
            x_speed = sqrt(stossfaktor) * x_speed;
        }

        if(y > (800)/scale - ballRadius && ((y_speed > 0 && y_speed < 0.1)) ){
            y_speed = 0;
            x_speed = 0;
            x_acc = 0;
            y_acc = 0;
        }

        // Anzeigen
        BeginDrawing();
        if (verlaufWahl[0]!='j'){
            ClearBackground(BLACK);
        }
        DrawCircle(x * scale, y * scale, ballRadius * scale, ballColor);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}