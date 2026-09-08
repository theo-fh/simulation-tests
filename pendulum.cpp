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
    int length; //nur für darstellung
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
        float density = 15.0f; //quasi die Dichte
        length = mass * density;
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
    //float phi; //drehwinkel
    //float omega; //drehgeschwindigkeit - nicht nötig
    PointMass* connected_mass;
    int force_index_x; //index in Kräfte-"Vektoren" der verbundenen Masse (ist die Menge der einwirkenden Kräfte jeweils in x- und y-Richtung)
    int force_index_y;


    RodFix(float fixposX_given, float fixposY_given, PointMass* connected_mass_given){
        length = (float) sqrt(pow(connected_mass_given->pos_x - fixposX_given, 2) + pow(connected_mass_given->pos_y - fixposY_given, 2));
        //phi = atan((connected_mass_given->pos_y - fixposY_given)/(connected_mass_given->pos_x - fixposX_given));
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
        float line_thickness = 5.0f;
        DrawLineEx({fix_pos_x, fix_pos_y}, {connected_mass->pos_x, connected_mass->pos_y}, line_thickness, BLACK);
    }

    void update(){
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
    }


    void correctPosition(){
        float dx = connected_mass->pos_x - fix_pos_x;
        float dy = connected_mass->pos_y - fix_pos_y;
        float current_length = sqrtf(dx * dx + dy * dy);

        if (current_length < 1e-6f) return;

        float nx = dx / current_length;
        float ny = dy / current_length;

        connected_mass->pos_x = fix_pos_x + nx * length;
        connected_mass->pos_y = fix_pos_y + ny * length;
    }

    void correctVelocity(){
        float dx = connected_mass->pos_x - fix_pos_x;
        float dy = connected_mass->pos_y - fix_pos_y;
        float current_length = sqrtf(dx * dx + dy * dy);

        if (current_length < 1e-6f) return;

        float nx = dx / current_length;
        float ny = dy / current_length;

        float radial_velocity =
            connected_mass->vel_x * nx +
            connected_mass->vel_y * ny;

        connected_mass->vel_x -= radial_velocity * nx;
        connected_mass->vel_y -= radial_velocity * ny;
    }
};

class RodConnect{
    public:
    float length;
    float force;
    PointMass* connected_mass_1;
    int fx_index_mass1;
    int fy_index_mass1;
    PointMass* connected_mass_2;
    int fx_index_mass2;
    int fy_index_mass2;
    float reduced_mass;

    RodConnect(PointMass* mass1_given, PointMass* mass2_given){
        connected_mass_1 = mass1_given;
        connected_mass_2 = mass2_given;
        length = (float) sqrt(pow(mass2_given->pos_x - mass1_given->pos_x, 2) + pow(mass2_given->pos_y - mass1_given->pos_y, 2));
        
        fx_index_mass1 = connected_mass_1->forces_x.size();
        fy_index_mass1 = connected_mass_1->forces_y.size();
        connected_mass_1->forces_x.push_back(0.0f);
        connected_mass_1->forces_y.push_back(0.0f);

        fx_index_mass2 = connected_mass_2->forces_x.size();
        fy_index_mass2 = connected_mass_2->forces_y.size();
        connected_mass_2->forces_x.push_back(0.0f);
        connected_mass_2->forces_y.push_back(0.0f);

        reduced_mass = (connected_mass_1->mass * connected_mass_2->mass) / (connected_mass_1->mass + connected_mass_2->mass);
    }
    
    void draw(){
        float line_thickness = 5.0f;
        DrawLineEx({connected_mass_2->pos_x, connected_mass_2->pos_y}, {connected_mass_1->pos_x, connected_mass_1->pos_y}, line_thickness, BLACK);
    }

    void update(){
        float other_forces_x1 = 0.0f; //Auf Masse 1 bezogen
        float other_forces_y1 = 0.0f;

        float other_forces_x2 = 0.0f; //Auf Masse 2 bezogen
        float other_forces_y2 = 0.0f;

        float nx = (connected_mass_2->pos_x - connected_mass_1->pos_x) / length;
        float ny = (connected_mass_2->pos_y - connected_mass_1->pos_y) / length;

        //andere Kräfte aufaddieren
        for(int i = 0; i < connected_mass_1->forces_x.size(); i++) if(i!=fx_index_mass1) other_forces_x1 += connected_mass_1->forces_x[i];
        for(int i = 0; i < connected_mass_1->forces_y.size(); i++) if(i!=fy_index_mass1) other_forces_y1 += connected_mass_1->forces_y[i];


        for(int i = 0; i < connected_mass_2->forces_x.size(); i++) if(i!=fx_index_mass2) other_forces_x2 += connected_mass_2->forces_x[i];
        for(int i = 0; i < connected_mass_2->forces_y.size(); i++) if(i!=fy_index_mass2) other_forces_y2 += connected_mass_2->forces_y[i];

        float v_radial_rel = (connected_mass_2->vel_x - connected_mass_1->vel_x) * nx + (connected_mass_2->vel_y - connected_mass_1->vel_y) * ny; //soll 0 sein
        float v_tang_rel = (connected_mass_2->vel_x - connected_mass_1->vel_x) * ny - (connected_mass_2->vel_y - connected_mass_1->vel_y) * nx;

        //Von außen angreifende Kräfte mittels reduzierter Masse mü auf Stabkraft schließen, zusammen mit Zentripetalkraft
        float acc1_ext_radial = (other_forces_x1 / connected_mass_1->mass) * nx + (other_forces_y1 / connected_mass_1->mass) * ny;
        float acc2_ext_radial = (other_forces_x2 / connected_mass_2->mass) * nx + (other_forces_y2 / connected_mass_2->mass) * ny;
        float force_external = reduced_mass * (acc2_ext_radial - acc1_ext_radial);

        float force_centripetal = reduced_mass * (v_tang_rel * v_tang_rel) / length;

        force = force_centripetal + force_external;

        //Stabkraft in Massen eintragen
        connected_mass_1->forces_x[fx_index_mass1] =  force * nx;
        connected_mass_1->forces_y[fy_index_mass1] =  force * ny;

        connected_mass_2->forces_x[fx_index_mass2] = -force * nx;
        connected_mass_2->forces_y[fy_index_mass2] = -force * ny;

        
    }

    void correctPosition(){
        float dx = connected_mass_2->pos_x - connected_mass_1->pos_x;
        float dy = connected_mass_2->pos_y - connected_mass_1->pos_y;
        float current_length = sqrtf(dx * dx + dy * dy);

        if (current_length < 1e-6f) return;

        float nx = dx / current_length;
        float ny = dy / current_length;

        float inverse_mass_1 = 1.0f / connected_mass_1->mass;
        float inverse_mass_2 = 1.0f / connected_mass_2->mass;
        float inverse_mass_sum = inverse_mass_1 + inverse_mass_2;

        float correction_factor = 0.8f;
        float error = (current_length - length) * correction_factor;
        float correction = error / inverse_mass_sum;

        connected_mass_1->pos_x += inverse_mass_1 * correction * nx;
        connected_mass_1->pos_y += inverse_mass_1 * correction * ny;

        connected_mass_2->pos_x -= inverse_mass_2 * correction * nx;
        connected_mass_2->pos_y -= inverse_mass_2 * correction * ny;
    }

    void correctVelocity(){
        float dx = connected_mass_2->pos_x - connected_mass_1->pos_x;
        float dy = connected_mass_2->pos_y - connected_mass_1->pos_y;
        float current_length = sqrtf(dx * dx + dy * dy);

        if (current_length < 1e-6f) return;

        float nx = dx / current_length;
        float ny = dy / current_length;

        float relative_vx = connected_mass_2->vel_x - connected_mass_1->vel_x;
        float relative_vy = connected_mass_2->vel_y - connected_mass_1->vel_y;
        float radial_velocity = relative_vx * nx + relative_vy * ny;

        float inverse_mass_1 = 1.0f / connected_mass_1->mass;
        float inverse_mass_2 = 1.0f / connected_mass_2->mass;
        float inverse_mass_sum = inverse_mass_1 + inverse_mass_2;

        float impulse = radial_velocity / inverse_mass_sum;

        connected_mass_1->vel_x += inverse_mass_1 * impulse * nx;
        connected_mass_1->vel_y += inverse_mass_1 * impulse * ny;

        connected_mass_2->vel_x -= inverse_mass_2 * impulse * nx;
        connected_mass_2->vel_y -= inverse_mass_2 * impulse * ny;
    }

};

void getInitValues(bool ask, float bearing_x, float bearing_y, float& mass1_pos_x, float& mass1_pos_y, float& mass2_pos_x, float& mass2_pos_y) {
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
        angle_deg = 75.0f;
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
    float fps = 1.0f/frametime;
    int fps_round = (int) std::round(fps);
    std::string text_fps = std::to_string(fps_round) + " FPS";

    DrawTextEx(font_given, text_fps.c_str(), position, 20.0f, 0.0f, color);
}

int main(){

    //Lager bestimmen
    int win_dimensions[] = {600, 500};
    float bearing_x = (float) win_dimensions[0]/2.0f;
    float bearing_y = 60.0f;

    //erste Punktmasse initiieren
    float acceleration_g = 150.0f;

    //Masse nach Länge und Auslenkung einführen
    float mass1_init_x;
    float mass1_init_y;
    float mass2_init_x;
    float mass2_init_y;

    getInitValues(false, bearing_x, bearing_y, mass1_init_x, mass1_init_y, mass2_init_x, mass2_init_y);
    
    PointMass mass1("m1", 2.5f, mass1_init_x, mass1_init_y, acceleration_g);
    PointMass mass2("m2", 3.0f, mass2_init_x, mass2_init_y, acceleration_g);

    //Lager und Masse mit Stab verbinden
    RodFix rod1(bearing_x, bearing_y, &mass1);
    RodConnect rod2(&mass1, &mass2);

    //Fenster initiieren u.ä.
    InitWindow(win_dimensions[0], win_dimensions[1], "Spring-Damper System");
    Font notoserif = LoadFontEx("fonts/NotoSerif-VariableFont_wdth,wght.ttf", 20, NULL, 0);
    Texture2D bearingImg = LoadTexture("images/bearing.png");
    int bearingImgWidth = 48;
    int bearingImgCircleHeight = 44;
    int targetFPS = 60;
    float speed_factor = 2.5f; //lässt die simulation im zeitraffer ablaufen
    SetTargetFPS(targetFPS);

    while(!WindowShouldClose()){
        float frame_dt = GetFrameTime();

        //Numerische integration führt bei hohen Auslenkungen zu fehlern, die durch ein kleineres dt minimiert, aber nie eliminiert werden können.
        int sub_steps = 20;

        float dt = (frame_dt * speed_factor) / (float)sub_steps;
        //Position erneuern

        for(int step = 0; step < sub_steps; step++){
            rod1.update();
            rod2.update();

            mass1.updateAcc();
            mass2.updateAcc();

            mass1.updateVel(dt);
            mass2.updateVel(dt);

            mass1.updatePos(dt);
            mass2.updatePos(dt);
            
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
        rod1.draw();
        rod2.draw();
        mass1.drawAsCircle(notoserif);
        mass2.drawAsCircle(notoserif);
        DrawTexture(bearingImg, (int) bearing_x - bearingImgWidth/2, (int) bearing_y - bearingImgCircleHeight, WHITE);
        EndDrawing();
    }

    return 0;
}