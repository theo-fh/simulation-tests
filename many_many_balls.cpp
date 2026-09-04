#include <raylib.h>
#include <cmath>
#include <vector>
#include <random>
#include <iostream>


class Ball {
    public:
    float pos_x;
    float pos_y;
    float speed_x;
    float speed_y;
    float acc_x = 0.0; //zusätzliche beschleunigung, soll auch globale beschleunigung geben
    float acc_y = 0.0;

    float radius;
    float mass;
    Color color;
    bool collided_this_frame = false;

    Ball(float x_init,float y_init, float x_speed_init, float y_speed_init, float radius_init, float mass_init, Color color_init){
        pos_x = x_init;
        pos_y = y_init;
        speed_x = x_speed_init;
        speed_y = y_speed_init;
        radius = radius_init;
        mass = mass_init;
        color.r = color_init.r;
        color.g = color_init.g;
        color.b = color_init.b;
        color.a = color_init.a;
    }

    void updatePos(float dt){
        pos_x += speed_x * dt;
        pos_y += speed_y * dt;
    }

    bool freezeIfSlow(float window_height, float min_vel_squared, float acc_compensation_y){
        if(speed_x*speed_x + speed_y*speed_y < min_vel_squared && pos_y > window_height - radius){
            speed_x = 0;
            speed_y = 0;
            acc_y = -acc_compensation_y;
            return true;
        }
        acc_y = 0.0f;
        return false;
    }

    void updateSpeed(float global_acc_x, float global_acc_y, float dt){
        speed_x += (global_acc_x + acc_x) * dt;
        speed_y += (global_acc_y + acc_y) * dt;
    }

    void draw(){
        DrawCircle(pos_x, pos_y, radius, color);
    }

    bool WallCollision(int window_width, int window_height){
        float wallCollisionFactor = 0.4;
        float floorSlowdownFactor = 0.7;
        if(pos_x > (float) window_width - radius && speed_x > 0){
            speed_x *= -wallCollisionFactor;
            collided_this_frame = true;
            pos_x = (float) window_width - radius;
            return true;
        }
        if(pos_x < 0.0f + radius && speed_x < 0){
            speed_x *= -wallCollisionFactor;
            collided_this_frame = true;
            pos_x = radius;
            return true;
        }
        if(pos_y > (float) window_height - radius && speed_y > 0){
            speed_y *= -wallCollisionFactor;
            speed_x *= floorSlowdownFactor;
            collided_this_frame = true;
            pos_y=(float) window_height - radius;
            return true;
        }
        /*
        if(pos_y < 0.0f + radius && speed_y < 0){
            speed_y *= -wallCollisionFactor;
            speed_x *= floorSlowdownFactor;
            collided_this_frame = true;
            return true;
        }
        */
    return false;
    }

};

bool checkBallCollision(Ball ball1, Ball ball2){

    //Vektoren aufstellen: weg von ball1 zu ball2 und relative geschwindigkeit mit ball2 als ruhend im bezugssystem
    float dx = ball2.pos_x - ball1.pos_x;
    float dy = ball2.pos_y - ball1.pos_y;
    float dvx = ball2.speed_x - ball1.speed_x;
    float dvy = ball2.speed_y - ball1.speed_y;

    //skalarprodukt muss positiv sein, dann stimmen die winkel
    if(dx*dx + dy*dy > (ball1.radius+ball2.radius)*(ball1.radius+ball2.radius)) return false;
    else return (dx * dvx + dy * dvy) < 0.0f;
}

void ballCollision(Ball& ball1, Ball& ball2, float friction_coeff){
    //erstmal nur vollelastischer stoß
    float dx = ball2.pos_x - ball1.pos_x;
    float dy = ball2.pos_y - ball1.pos_y;
    float dvx = ball2.speed_x - ball1.speed_x;
    float dvy = ball2.speed_y - ball1.speed_y;

    //normalenvektor aufstellen
    float distance = (float) sqrt(dx*dx + dy*dy);
    if(distance == 0) return;

    float nx = dx / distance;
    float ny = dy / distance;
    float tx = -ny;
    float ty = nx;

    //impulsskalar J normal und tangential aufstellen:
    float impulseJ = 2*(dvx * nx + dvy * ny)/(1 / ball1.mass + 1 / ball2.mass);
    float impulseJtang = (dvx * tx + dvy * ty)/(1 / ball1.mass + 1 / ball2.mass);
    
    //maximale reibung: tangentialer impuls kann nicht höher sein als diese
    float max_friction = friction_coeff * abs(impulseJ);

    if(impulseJtang>max_friction) impulseJtang = max_friction;
    if(impulseJtang<-max_friction) impulseJtang = -max_friction;

    //Geschwindigkeitsänderung eintragen
    ball1.speed_x += (impulseJ * nx + impulseJtang * tx) / ball1.mass;
    ball1.speed_y += (impulseJ * ny + impulseJtang * ty) / ball1.mass;

    ball2.speed_x -= (impulseJ * nx + impulseJtang * tx) / ball2.mass;
    ball2.speed_y -= (impulseJ * ny + impulseJtang * ty) / ball2.mass;

    //Überlappung verhindern, damit die Kugeln nicht ineinander versinken
    float overlap = ball1.radius + ball2.radius - distance;
    if(overlap > 0.0f){
        float separation_factor = 0.8f; //pro frame nur 80% der überlappung beheben → gegen zittern
        float separation_x = nx * overlap * separation_factor;
        float separation_y = ny * overlap * separation_factor;

        ball1.pos_x -= separation_x * 0.5f; //Faktor 0.5 damit beide Kugeln jeweils eine Hälfte des Overlaps übernehmen
        ball1.pos_y -= separation_y * 0.5f;

        ball2.pos_x += separation_x * 0.5f;
        ball2.pos_y += separation_y * 0.5f;

    }
}

static inline std::mt19937 gen{ std::random_device{}() };

int main(){


    //Fenster initiieren
    int width = 800;
    int height = 800;
    InitWindow(width, height, "Many many balls");
    int targetFPS = 60;
    SetTargetFPS(targetFPS);
    float acceleration_factor = 2; //macht alle vorgänge schneller
    float dt = (float) acceleration_factor* 1.0/targetFPS;
    
    
    //Variablen bestimmen
    float g = 40;


    //Kugeln hinzufügen (zufällig)
    std::uniform_real_distribution<float> start_x(60.0f, width - 60.0f);
    std::uniform_real_distribution<float> start_y(60.0f, height - 150.0f);
    std::uniform_real_distribution<float> start_vel(-100.0f, 100.0f);
    std::uniform_real_distribution<float> start_radius(10.0f, 30.0f);
    std::uniform_real_distribution<float> start_mass(1.0f, 5.0f);
    std::uniform_int_distribution<int> color_chooser(0,5);

    int anzahl_baelle = 100;
    std::vector<Ball> balls;
    for(int i = 0; i < anzahl_baelle; i++){
        Color colors[] = {RED, GREEN, BLUE, PURPLE, BEIGE, GOLD};
        balls.push_back(Ball(start_x(gen), start_y(gen), start_vel(gen), start_vel(gen), start_radius(gen), start_mass(gen), colors[color_chooser(gen)]));
    }
    int ballsAmount = size(balls);


    //Game Loop
    while(WindowShouldClose() == false){
        
        //Kollisionen überprüfen
        for(int i=0; i<ballsAmount; i++){
            
            
            //if(balls[i].collided_this_frame) break;
            for(int j=0; j<ballsAmount; j++){
                if(i != j){   
                    if(checkBallCollision(balls[i], balls[j])){
                        ballCollision(balls[i], balls[j], 0.2f);
                        balls[i].collided_this_frame = true;
                        balls[j].collided_this_frame = true;
                        break;
                    }
                }
            }

            balls[i].WallCollision(width, height);
        }


        //Position updaten
        float min_velocity = 18.0f; //für freezeIfSlow

        for(int i=0; i<ballsAmount; i++){
            balls[i].freezeIfSlow(height, min_velocity, g);
            balls[i].updatePos(dt);
            balls[i].updateSpeed(0,g,dt);
        }

        //std::cout << balls[10].speed_y << " "; //zum Debugging

        //Anzeige erneuern
        BeginDrawing();
        ClearBackground(BLACK);
        for(int i=0; i<ballsAmount; i++){
            balls[i].draw();
        }
        EndDrawing();

    }

    CloseWindow();
    return 0;
}