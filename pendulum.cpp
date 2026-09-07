#include <raylib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <cmath>
#include <vector>

class PointMass{
    public:
    std::string name;
    float mass;
    int length = 60; //nur für darstellung
    float pos_x;
    float pos_y;
    float vel_x = 0.0f;
    float vel_y = 0.0f;
    float acc_x = 0.0f;
    float acc_y = 0.0f;
    float line_thickness = 3.0f;
    std::vector<float> forces_x;
    std::vector<float> forces_y;


    PointMass(std::string name_given, float mass_given, float posx_given, float posy_given, float g){
        name = name_given;
        mass = mass_given;
        pos_x = posx_given;
        pos_y = posy_given;
        forces_y.push_back(mass_given * g);
    }

    void drawAsCircle(Font font_given){
        float draw_x = std::round(pos_x);
        float draw_y = std::round(pos_y);

        DrawCircle(draw_x, draw_y, length/2.0, GRAY);
        DrawCircleLinesEx({draw_x, draw_y}, length/2.0, line_thickness, BLACK);
        Vector2 textSize = MeasureTextEx(font_given, name.c_str(), 20.0f, 0.0f);
        DrawTextEx(font_given, name.c_str(), {std::round(pos_x - textSize.x / 2.0f), std::round(pos_y - textSize.y / 2.0f)}, 20.0f, 0.0f, BLACK);
    }

    void drawAsSquare(Font font_given){
        float draw_x = std::round(pos_x);
        float draw_y = std::round(pos_y);

        DrawRectangle(draw_x - length/2, pos_y - length/2, length, length, GRAY);
        DrawLineEx({draw_x - length/2 - line_thickness/2, draw_y - length/2}, {draw_x + length/2 + line_thickness/2, draw_y - length/2}, line_thickness, BLACK);
        DrawLineEx({draw_x - length/2, draw_y - length/2}, {draw_x - length/2, draw_y + length/2}, line_thickness, BLACK);
        DrawLineEx({draw_x + length/2, draw_y + length/2}, {draw_x + length/2, draw_y - length/2}, line_thickness, BLACK);
        DrawLineEx({draw_x + length/2 + line_thickness/2, draw_y + length/2}, {draw_x - length/2 - line_thickness/2, draw_y + length/2}, line_thickness, BLACK);

        Vector2 textSize = MeasureTextEx(font_given, name.c_str(), 20.0f, 0.0f);
        DrawTextEx(font_given, name.c_str(), {std::round(pos_x - textSize.x / 2.0f), std::round(pos_y - textSize.y / 2.0f)}, 20.0f, 0.0f, BLACK);
    }

    void updatePos(float dt){
        pos_x += vel_x * dt;
        pos_y += vel_y * dt;
    }

    void updateVel(float dt){
        vel_x += acc_x * dt;
        vel_y += acc_y * dt;
    }

    void updateAcc(){
        float force_sum_x = 0.0f;
        float force_sum_y = 0.0f;
        for(int i = 0; i < forces_x.size(); i++) force_sum_x += forces_x[i];
        for(int i = 0; i < forces_y.size(); i++) force_sum_y += forces_y[i];

        acc_x = force_sum_x / mass;
        acc_y = force_sum_y / mass;
    }
};

class RodFix {
    //masseloser Stab, fürs erste immer fest gelagert
    public:
    float length;
    float fix_pos_x;
    float fix_pos_y;
    float force;
    float phi; //drehwinkel
    //float omega; //drehgeschwindigkeit - nicht nötig
    PointMass* connected_mass;
    int force_index_x; //index in Kräfte-"Vektoren" der verbundenen Masse (ist die Menge der einwirkenden Kräfte jeweils in x- und y-Richtung)
    int force_index_y;
    float line_thickness = 5.0f;

    RodFix(float fixposX_given, float fixposY_given, PointMass* connected_mass_given){
        length = (float) sqrt(pow(connected_mass_given->pos_x - fixposX_given, 2) + pow(connected_mass_given->pos_y - fixposY_given, 2));
        phi = atan((connected_mass_given->pos_y - fixposY_given)/(connected_mass_given->pos_x - fixposX_given));
        fix_pos_x = fixposX_given;
        fix_pos_y = fixposY_given;
        connected_mass = connected_mass_given;

        //einfügen der stabkraft in die Daten der verbundenen Masse, x und y Richtung
        force_index_x = connected_mass->forces_x.size();
        connected_mass->forces_x.push_back(0.0f);

        force_index_y = connected_mass->forces_y.size();
        connected_mass->forces_y.push_back(0.0f);
    }
        

    void draw(){
        DrawLineEx({fix_pos_x, fix_pos_y}, {connected_mass->pos_x, connected_mass->pos_y}, line_thickness, BLACK);
    }

    void update(){
        //phi = (float) atan2(connected_mass->pos_y - fix_pos_y, connected_mass->pos_x - fix_pos_x);
        //float sin_phi = (float) sin(phi);
        //float cos_phi = (float) cos(phi);

        float other_forces_x = 0.0f; //Summe der außer dem Stab einwirkenden Kräfte auf die verbundene Masse
        float other_forces_y = 0.0f;
    
        float nx = (connected_mass->pos_x - fix_pos_x) / length;
        float ny = (connected_mass->pos_y - fix_pos_y) / length;
        
        //andere Kräfte aufaddieren
        for(int i = 0; i < connected_mass->forces_x.size(); i++) if(i!=force_index_x) other_forces_x += connected_mass->forces_x[i];
        for(int i = 0; i < connected_mass->forces_y.size(); i++) if(i!=force_index_y) other_forces_y += connected_mass->forces_y[i];

        float v_radial = connected_mass->vel_x * nx + connected_mass->vel_y * ny;
        float v_tangent_sq = (connected_mass->vel_x * connected_mass->vel_x + connected_mass->vel_y * connected_mass->vel_y) - (v_radial * v_radial);
        if (v_tangent_sq < 0.0f) v_tangent_sq = 0.0f;

        

        float force_external = nx * other_forces_x + ny * other_forces_y;
        float force_centripetal = connected_mass->mass * v_tangent_sq / length;
        force = force_external + force_centripetal;

        //Stabkraft wieder zerlegen und in Objekt verbundene Masse eintragen
        connected_mass->forces_x[force_index_x] = -force * nx;
        connected_mass->forces_y[force_index_y] = -force * ny;

        connected_mass->pos_x = fix_pos_x + nx * length;
        connected_mass->pos_y = fix_pos_y + ny * length;

        connected_mass->vel_x -= v_radial * nx;
        connected_mass->vel_y -= v_radial * ny;
    }
};

void getInitValues(char ask, float bearing_x, float bearing_y, float& mass_x, float& mass_y) {
    float angle_deg;
    float length;
    
    if(ask == 'y'){
        std::cout << "Auslenkung (Grad): ";
        float angle_deg;
        float length;

        // Fängt fehlerhafte Eingaben ab
        while (!(std::cin >> angle_deg)) {
            std::cout << "Ungueltige Eingabe.";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }

        std::cout << "Länge (Pixel): ";
        while (!(std::cin >> length)) {
            std::cout << "Ungueltige Eingabe.";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
    }

    else {
        angle_deg = 30.0f;
        length = 350.0f;
    }
    float angle = angle_deg * PI / 180;
    mass_x = (float) bearing_x + sin(angle) * length;
    mass_y = (float) bearing_y + cos(angle) * length;
}

int main(){

    //Lager bestimmen
    int win_dimensions[] = {600, 500};
    float bearing_x = (float) win_dimensions[0]/2.0f;
    float bearing_y = 60.0f;

    //erste Punktmasse initiieren
    float acceleration_g = 200.0f;

    //Masse nach Länge und Auslenkung einführen
    float mass_init_x;
    float mass_init_y;
    getInitValues('n', bearing_x, bearing_y, mass_init_x, mass_init_y);
    
    PointMass mass1("m1", 2.0f, mass_init_x, mass_init_y, acceleration_g);

    //Lager und Masse mit Stab verbinden
    RodFix rod1(bearing_x, bearing_y, &mass1);

    //Fenster initiieren u.ä.
    InitWindow(win_dimensions[0], win_dimensions[1], "Spring-Damper System");
    Font notoserif = LoadFontEx("fonts/NotoSerif-VariableFont_wdth,wght.ttf", 20, NULL, 0);
    Texture2D bearingImg = LoadTexture("images/bearing.png");
    int bearingImgWidth = 48;
    int bearingImgCircleHeight = 44;
    int targetFPS = 60;
    float speed_factor = 3.0f; //lässt die simulation im zeitraffer ablaufen
    SetTargetFPS(targetFPS);

    while(!WindowShouldClose()){
        float dt = speed_factor * GetFrameTime();
        //Position erneuern
        rod1.update();
        mass1.updateAcc();
        mass1.updateVel(dt);
        mass1.updatePos(dt);
        


        //Zeichnen
        BeginDrawing();
        ClearBackground(WHITE);
        rod1.draw();
        mass1.drawAsCircle(notoserif);

        DrawTexture(bearingImg, (int) bearing_x - bearingImgWidth/2, (int) bearing_y - bearingImgCircleHeight, WHITE);
        EndDrawing();
    }

    return 0;
}