#pragma once

#include <iostream>
#include <cmath>
#include <raylib.h>
#include <vector>
#include "pendulum-objects.h"

enum operation{
    Excite,
    Suppress,
    Stabilize,
    Stop,
    Center
};

void printOperation(operation currOp, const Font font, const int fontsize){
    constexpr int posX = 30;
    constexpr int posY = 30;
    std::string opText;
    switch (currOp){
        case Excite: opText = "Excite"; break;
        case Suppress: opText = "Suppress"; break;
        case Stabilize: opText = "Stabilize"; break;
        case Stop: opText = "Stop"; break;
        case Center: opText = "Center"; break;
        default: opText = "none";
    }
    std::string output = "Curr. Operation: " + opText;
    DrawTextEx(font, output.c_str(), {posX, posY}, fontsize, 0.0f, BLACK);
}

void drawForcePush(PointMass* mass, const float nx, const float ny, const float amount, const float scale){
    float start_x = mass->pos_x - nx * mass->length / 2.0f;
    float start_y = mass->pos_y - ny * mass->length / 2.0f;

    float end_x = start_x - nx * amount * scale;
    float end_y = start_y - ny * amount * scale;

    constexpr float thickness = 3.0f;
    DrawLineEx({start_x, start_y}, {end_x, end_y}, thickness, RED);
    constexpr float arrow_length = 16.0f;
    constexpr float arrow_thick = 7.0f;

    float point3_x = start_x - nx * arrow_length - ny * arrow_thick;
    float point3_y = start_y - ny * arrow_length + nx * arrow_thick;

    float point2_x = start_x - nx * arrow_length + ny * arrow_thick;
    float point2_y = start_y - ny * arrow_length - nx * arrow_thick;
    DrawTriangle({start_x, start_y}, {point2_x, point2_y}, {point3_x, point3_y}, RED);
}

bool detectEquilibrium(PointMass* mass, Motor* motor, const float tolerance_factor){
    float rel_pos = motor->pos_x - mass->pos_x;
    //TODO: optional Erkennung anpassen, dass sie bei hohen geschwindigkeiten nicht aussetzt
    //float wiggleroom = mass->vel_x * tolerance_factor + 1.0f; //+ 1.0f damit er auslöst wenn das pendel ruht
    float wiggleroom = 10.0f;
    if(rel_pos < wiggleroom && rel_pos > -wiggleroom) return true;
    return false;
}

void getInitValues(bool ask, float bearing_x, float bearing_y, float& mass1_pos_x, float& mass1_pos_y, float& mass2_pos_x, float& mass2_pos_y) {
    //fragt Nutzer Länge der einzelnen Pendel und Ausschlag (gleicher Winkel für beide Gelenke), rechnet diese in kartesische Koordinaten für
    //die Ausgangsposition der Massen um
    float angle_deg;
    float length1;
    float length2;
    
    if(ask == true){
        std::cout << "Auslenkung (Grad): ";

        // Fängt fehlerhafte Eingaben ab
        while (!(std::cin >> angle_deg)) {
            std::cout << "Ungültige Eingabe.";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }

        std::cout << "Länge erstes Pendel (px): ";
        while (!(std::cin >> length1)) {
            std::cout << "Ungültige Eingabe.";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }

        std::cout << "Länge zweites Pendel (px): ";
        while (!(std::cin >> length2)) {
            std::cout << "Ungültige Eingabe.";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
    }

    else {
        angle_deg = 80.0f;
        length1 = 180.0f;
        length2 = 100.0f;
    }
    float angle = angle_deg * PI / 180;
    mass1_pos_x = (float) bearing_x + sin(angle) * length1;
    mass1_pos_y = (float) bearing_y + cos(angle) * length1;
    mass2_pos_x = mass1_pos_x + sin(angle) * length2;
    mass2_pos_y = mass1_pos_y + cos(angle) * length2;

}

void printFPS(Font font_given, float frametime, Vector2 position, Color color){
    //bildet aktuelle Bildrate im Fenster ab
    float fps = 1.0f/frametime;
    int fps_round = (int) std::round(fps);
    std::string text_fps = std::to_string(fps_round) + " FPS";

    DrawTextEx(font_given, text_fps.c_str(), position, 20.0f, 0.0f, color);
}

void traceMass(PointMass* mass, std::vector<Vector2> &trajectory){
    //zeichnet den Positionsverlauf der angegebenen Punktmasse an
    Vector2 position = {mass->pos_x, mass->pos_y};
    trajectory.push_back(position);

    int length = 200;
    if(trajectory.size() > length) trajectory.erase(trajectory.begin());

    DrawLineStrip(trajectory.data(), static_cast<int>(trajectory.size()), GREEN);
}

