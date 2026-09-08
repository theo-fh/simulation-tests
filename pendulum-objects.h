#pragma once

#include <raylib.h>
#include <string.h>
#include <cmath>
#include <vector>

class PointMass{
    //Punktmasse, kann beliebig viele angreifende Kräfte besitzen
    public:
    std::string name;
    float mass;
    int length; //Kantenlänge bzw. Durchmesser, nur für Darstellung
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

    //kann auch als Quadrat gezeichnet werden, nützlich, um die Class in anderen Programmen wiederzuverwenden
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
    //masseloser Stab, der ein Festlager und eine Masse verbindet
    public:
    float length;
    float fix_pos_x;
    float fix_pos_y;
    float force;
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
    //masseloser Stab, der zwei Massen miteinander verbindet
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
    float inverse_mass_sum;
    float inverse_mass_1;
    float inverse_mass_2;

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
        inverse_mass_1 = 1.0f / connected_mass_1->mass;
        inverse_mass_2 = 1.0f / connected_mass_2->mass;
        inverse_mass_sum = inverse_mass_1 + inverse_mass_2;
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

        float impulse = radial_velocity / inverse_mass_sum;

        connected_mass_1->vel_x += inverse_mass_1 * impulse * nx;
        connected_mass_1->vel_y += inverse_mass_1 * impulse * ny;

        connected_mass_2->vel_x -= inverse_mass_2 * impulse * nx;
        connected_mass_2->vel_y -= inverse_mass_2 * impulse * ny;
    }

};