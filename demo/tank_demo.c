#include "sdl_wrapper.h"
#include "polygon.h"
#include "vector.h"
#include "scene.h"
#include "color.h"
#include "forces.h"
#include "body.h"
#include "tank.h"
#include "terrain.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>

// Cartesian Coordinates (not pixel values)
const vector_t MIN = {0.0, 0.0};
const vector_t MAX = {200.0, 100.0};

const double PI = 3.14159265;

const double TANK_SIZE = 5.0;
const vector_t TANK_VELO = {100.0, 0};

const rgb_color_t BLACK = {0.0, 0.0, 0.0};
const double DAMAGE = 25.0; //will need to update how this works with variable bullet types
const double BULLET_MASS = 1.0;
const double BULLET_SIZE = 2.0;
const double G = 690.0;

const double BASE_POWER = 10.0;

const double BASE_HEIGHT = 30.0;
const double TERRAIN_SCALE = 20.0;
const int NUM_TERRAIN_LEVELS = 7; // This can only be 1 for now
const double TERRAIN_DAMPING = 0.5;
const double TERRAIN_MASS = 10.0;
const double TERRAIN_AMPLITUDE = 1.0;

const double BIG_MASS = 100000000.0;
const int ANCHOR_OFF = 100000;
const int ANCHOR_HEIGHT = 10;

const int GAME_LEVELS = 2;
const rgb_color_t LIGHT_BLUE = {173.0 / 255.0, 216.0 / 255.0, 230.0 / 255.0};

typedef struct bullet_info {
    double damage;
    //Variable bullet types can be added here
} bullet_info_t;

void shoot_bullet(scene_t *scene, body_t *tank) {
    list_t *points = create_arc(BULLET_SIZE, 2*PI);
    bullet_info_t *bullet_aux = malloc(sizeof(bullet_info_t));
    bullet_aux->damage = DAMAGE;
    body_t *bullet = body_init_with_info(points, BULLET_MASS, BLACK, (void *) bullet_aux, free);
    vector_t tank_center = body_get_centroid(tank);
    body_set_centroid(bullet, tank_center);

    double angle = tank_get_angle(tank);
    angle = (angle * PI) / 180.0;
    double x_dir = cos(angle);
    double y_dir = sin(angle);

    double power = tank_get_power(tank);
    power = power + BASE_POWER;
    vector_t velo = vec_multiply(power, (vector_t) {x_dir, y_dir});

    int type = tank_get_number(tank);
    if (type == 1) {
        body_set_velocity(bullet, velo);
    }
    else {
        //If the tank is on the righthand side then the x-velo needs to be flipped
        velo.x = velo.x * -1;
        body_set_velocity(bullet, velo);
    }
    scene_add_body(scene, bullet);
    create_newtonian_gravity(scene, G, bullet, scene_get_body(scene, 10)); //Tenth indexed body in scene is ground

    body_t *other_tank;
    if (type == 1) {
        //Second body in scene is tank 2
        other_tank = scene_get_body(scene, 2);
    }
    else {
        //Second body in scene is tank 1
        other_tank = scene_get_body(scene, 1);
    }

    create_damaging_collision(scene, other_tank, bullet);
    tank_set_turn(other_tank, true);

    body_t *terrain = scene_get_body(scene, 9); //this is the terrain index
    // create_oneway_destructive_collision(scene, terrain, bullet);
    create_bullet_destroy(scene, terrain, bullet);
    tank_set_turn(tank, false);

}

body_t *tank_turn(scene_t *scene) {
    body_t *tank1 = scene_get_body(scene, 1);
    bool turn1 = tank_get_turn(tank1);
    if (turn1) {
        return tank1;
    }
    body_t *tank2 = scene_get_body(scene, 2);
    return tank2;
}

void tank_wall_collision(body_t *tank) {
    list_t *shape = body_get_shape(tank);
    vector_t velo = body_get_velocity(tank);
    int tank_num = tank_get_number(tank);
    for (size_t i = 0; i < list_size(shape); i++) {
        vector_t *current_vec = list_get(shape, i);
        if (tank_num == 1) {
            if ((current_vec->x > MAX.x && velo.x > 0) || (current_vec->x < MIN.x && velo.x < 0)) {
                velo.x = 0.0;
            }
        }

        else if (tank_num == 2) {
            if ((current_vec->x > MAX.x && velo.x > 0) || (current_vec->x < MIN.x && velo.x < 0)) {
                velo.x = 0.0;
            }
        }
    }
    body_set_velocity(tank, velo);
    list_free(shape);
}

void render_tank(body_t *tank, double angle, double power, vector_t velocity, double held_time) {
    double current_angle = tank_get_angle(tank);
    double current_power = tank_get_power(tank);

    tank_set_angle(tank, current_angle+angle);
    tank_set_power(tank, current_power+power);

    body_set_velocity(tank, velocity);
    tank_wall_collision(tank);
}

void shooter_key_handler(char key, key_event_type_t type, double held_time, scene_t *scene) {
    body_t *tank = tank_turn(scene);
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                render_tank(tank, 0.0, 0.0, (vector_t) {-TANK_VELO.x, TANK_VELO.y}, held_time);
                break;

            case RIGHT_ARROW:
                render_tank(tank, 0.0, 0.0, (vector_t) {TANK_VELO.x, TANK_VELO.y}, held_time);
                break;

            case UP_ARROW:
                if (tank_get_angle(tank) > 89) {
                    render_tank(tank, 0.0, 0.0, (vector_t){ 0 , 0 }, held_time);
                }
                else {
                    render_tank(tank, 1.0, 0.0, (vector_t){ 0 , 0 }, held_time);
                }
                break;

            case DOWN_ARROW:
                if (tank_get_angle(tank) < -89) {
                    render_tank(tank, 0.0, 0.0, (vector_t){ 0 , 0 }, held_time);
                }
                else {
                    render_tank(tank, -1.0, 0.0, (vector_t){ 0 , 0 }, held_time);
                }
                break;

            case W:
                if (tank_get_power(tank) > 99) {
                    render_tank(tank, 0.0, 0.0, (vector_t){ 0 , 0 }, held_time);
                }
                else {
                    render_tank(tank, 0.0, 1.0, (vector_t){ 0 , 0 }, held_time);
                }
                break;

            case S:
                if (tank_get_power(tank) < 1) {
                    render_tank(tank, 0.0, 0.0, (vector_t){ 0 , 0 }, held_time);
                }
                else {
                    render_tank(tank, 0.0, -1.0, (vector_t){ 0 , 0 }, held_time);
                }
                break;

            case SPACE_BAR:
                shoot_bullet(scene, tank);
        }
    }

    else if (type == KEY_RELEASED) {
        render_tank(tank, 0.0, 0.0, (vector_t){ 0 , 0 }, held_time);
    }
}

body_t *make_sky(rgb_color_t sky_color) {
    vector_t center = {(MAX.x / 2), (MAX.y / 2)};
    double height = MAX.y;
    double width = MAX.x;
    list_t *points = create_rectangle(center, width, height);
    body_t *rec = body_init(points, 1.0, sky_color);
    return rec;
}

body_t *make_ground(void) {
    list_t *list = list_init(4, (free_func_t) free);
    vector_t *p = malloc(sizeof(vector_t));
    *p = (vector_t){MIN.x, MIN.y - ANCHOR_OFF};
    list_add(list, p);

    vector_t *p1 = malloc(sizeof(vector_t));
    *p1 = (vector_t){MIN.x, MIN.y - (ANCHOR_OFF + ANCHOR_HEIGHT)};
    list_add(list, p1);

    vector_t *p2 = malloc(sizeof(vector_t));
    *p2 = (vector_t){MAX.x, MIN.y - (ANCHOR_OFF + ANCHOR_HEIGHT)};
    list_add(list, p2);

    vector_t *p3 = malloc(sizeof(vector_t));
    *p3 = (vector_t){MAX.x, MIN.y - ANCHOR_OFF };
    list_add(list, p3);

    rgb_color_t color = { 1, 1, 1 };
    return body_init(list, BIG_MASS, color);
}

scene_t *init_new_game() {
    scene_t *scene = scene_init();

    body_t *sky = make_sky(LIGHT_BLUE);
    scene_add_body(scene, sky);

    body_t *tank1 = tank_init(1.0, (rgb_color_t){0.0, 0.0, 0.0}, (vector_t) {20.0, 50.0}, TANK_SIZE, 1);
    scene_add_body(scene,tank1);
    body_t *tank2 = tank_init(1.0, (rgb_color_t){0.0, 0.5, 0.5}, (vector_t) {70.0, 50.0}, TANK_SIZE, 2);
    scene_add_body(scene,tank2);

    health_bar_t *health_bar1 = tank_get_health_bar(tank1);
    scene_add_body(scene, health_bar1->inner);
    scene_add_body(scene, health_bar1->outer);
    scene_add_body(scene, health_bar1->health_pool);
    health_bar_t *health_bar2 = tank_get_health_bar(tank2);
    scene_add_body(scene, health_bar2->inner);
    scene_add_body(scene, health_bar2->outer);
    scene_add_body(scene, health_bar2->health_pool);

    body_t *terrain = generate_terrain(MAX.x, BASE_HEIGHT, TERRAIN_SCALE, NUM_TERRAIN_LEVELS, TERRAIN_DAMPING, TERRAIN_MASS, TERRAIN_AMPLITUDE);
    scene_add_body(scene, terrain);
    create_terrain_follow(scene, terrain, tank1);
    create_terrain_follow(scene, terrain, tank2);
    create_health_follow(scene, tank1);
    create_health_follow(scene, tank2);

    body_t *ground = make_ground();
    scene_add_body(scene, ground);

    return scene;
}

int main() {
    scene_t *scene = init_new_game();

    sdl_init(MIN, MAX);
    sdl_on_key((key_handler_t) shooter_key_handler);

    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        scene_tick(scene, dt);
        sdl_render_scene(scene);

        body_t *tank1 = scene_get_body(scene, 1);
        body_t *tank2 = scene_get_body(scene, 2);
        int tank1_wins = 0;
        int tank2_wins = 0;

        if (tank_is_dead(tank1)) {
            printf("Player 2 wins!\n"); //Text rendering later?
            tank2_wins++;
            if (tank2_wins < GAME_LEVELS) {
                scene_free(scene);
                scene = init_new_game();
            }
            else {
                printf("Game over, Player 2 has won!\n");
            }
        }

        else if (tank_is_dead(tank2)) {
            printf("Player 1 wins!\n"); //Text rendering later?
            tank1_wins++;
            if (tank1_wins < GAME_LEVELS) {
                scene_free(scene);
                scene = init_new_game();
            }
            else {
                printf("Game over, Player 1 has won!\n");
            }
        }
    }
    scene_free(scene);
    return 0;
}
