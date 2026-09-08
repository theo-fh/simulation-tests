#pragma once

#include <iostream>
#include <cmath>
#include <raylib.h>
#include <vector>
#include "pendulum-objects.h"

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