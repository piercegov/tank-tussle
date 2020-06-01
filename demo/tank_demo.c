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
const double CIRC_DENSITY = 40.0;

const double TANK_SIZE = 20.0;
const vector_t TANK_VELO = {100.0, 0};

const rgb_color_t BLACK = {0.0, 0.0, 0.0};
const double DAMAGE = 25.0; //will need to update how this works with variable bullet types
const double BULLET_MASS = 1.0;
const double BULLET_SIZE = 2.0;
const double G = 0.1;

const double BASE_POWER = 10.0;

const double BASE_HEIGHT = 40.0;
const double TERRAIN_SCALE = 5.0;
const int NUM_TERRAIN_LEVELS = 1;
const double TERRAIN_DAMPING = 0.5;
const double TERRAIN_MASS = 10.0;

const double BIG_MASS = 1000000000000.0;
const int ANCHOR_OFF = 100000;
const int ANCHOR_HEIGHT = 10;

typedef struct bullet_info {
    double damage;
    //Variable bullet types can be added here
} bullet_info_t;

list_t *create_arc1(double d, double rads) {
    double total_pts = CIRC_DENSITY * rads;
    list_t *points = list_init(total_pts, (free_func_t) free);
    for (size_t i = 0; i < total_pts; i++) {
        vector_t *new_pt = malloc(sizeof(vector_t));
        *new_pt = vec_rotate((vector_t) {d / 2, 0}, i / CIRC_DENSITY);
        list_add(points, new_pt);
    }
    return points;
}

void shoot_bullet(scene_t *scene, body_t *tank) {
    list_t *points = create_arc1(BULLET_SIZE, 2*PI);
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
    vector_t velo = vec_multiply(power, (vector_t) {x_dir, y_dir});
    velo = vec_multiply(BASE_POWER, velo);

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
    create_newtonian_gravity(scene, G, tank, scene_get_body(scene, 4)); //Fourth body in scene is ground
    if (type == 1) {
        //Second body in scene is tank 2
        body_t *tank2 = scene_get_body(scene, 1);
        create_damaging_collision(scene, tank2, bullet);
        tank_set_turn(tank, false);
        tank_set_turn(tank2, true);
    }
    else {
        //Second body in scene is tank 1
        body_t *tank1 = scene_get_body(scene, 0);
        create_damaging_collision(scene, tank1, bullet);
        tank_set_turn(tank, false);
        tank_set_turn(tank1, true);
    }
}

body_t *tank_turn(scene_t *scene) {
    body_t *tank1 = scene_get_body(scene, 0);
    bool turn1 = tank_get_turn(tank1);
    if (turn1) {
        return tank1;
    }
    body_t *tank2 = scene_get_body(scene, 1);
    return tank2;
}

void render_tank(body_t *tank, double angle, double power, vector_t velocity, double held_time) {
    double current_angle = tank_get_angle(tank);
    double current_power = tank_get_power(tank);
    vector_t current_velo = body_get_velocity(tank);
    vector_t new_velocity;

    if (tank_get_number(tank) == 1) {
        new_velocity = vec_add(current_velo, vec_multiply(held_time, velocity));
    }

    else {
        new_velocity = vec_subtract(current_velo, vec_multiply(held_time, velocity));
    }

    tank_set_angle(tank, current_angle+angle);
    tank_set_power(tank, current_power+power);
    body_set_velocity(tank, new_velocity);
}

void shooter_key_handler(char key, key_event_type_t type, double held_time, scene_t *scene) {
    body_t *tank = tank_turn(scene);
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                if (tank_get_number(tank) == 1) {
                    render_tank(tank, 0.0, 0.0, (vector_t) {-TANK_VELO.x, TANK_VELO.y}, held_time);
                }
                else {
                    render_tank(tank, 0.0, 0.0, TANK_VELO, held_time);
                }
                break;

            case RIGHT_ARROW:
                if (tank_get_number(tank) == 2) {
                    render_tank(tank, 0.0, 0.0, (vector_t) {-TANK_VELO.x, TANK_VELO.y}, held_time);
                }
                else {
                    render_tank(tank, 0.0, 0.0, TANK_VELO, held_time);
                }
                break;

            case UP_ARROW:
                render_tank(tank, 1.0, 0.0, (vector_t){ 0 , 0 }, held_time);
                break;

            case DOWN_ARROW:
                render_tank(tank, -1.0, 0.0, (vector_t){ 0 , 0 }, held_time);
                break;

            case W:
                render_tank(tank, 0.0, 1.0, (vector_t){ 0 , 0 }, held_time);
                break;

            case S:
                render_tank(tank, 0.0, -1.0, (vector_t){ 0 , 0 }, held_time);
                break;

            case SPACE_BAR:
                shoot_bullet(scene, tank);
        }
    }

    else if (type == KEY_RELEASED) {
        render_tank(tank, 0.0, 0.0, (vector_t){ 0 , 0 }, held_time);
    }
}

// void tangent_bodies(body_t *tank, body_t *terrain) {
//     vector_t tank_center = body_get_centroid(tank);
//     vector_t terrain_max = body_get_max(terrain, tank_center.x - 1, tank_center.x + 1);
//     body_set_centroid(tank, (vector_t){ tank_center.x, terrain_max.y + TANK_SIZE });
// }

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

int main() {
    scene_t *scene = scene_init();

    body_t *tank1 = tank_init(1.0, (rgb_color_t){0.0, 0.0, 0.0}, (vector_t) {20.0, 50.0}, TANK_SIZE, 1);
    scene_add_body(scene,tank1);

    scene_add_body(scene, tank_get_health_bar(tank1)->inner);
    scene_add_body(scene, tank_get_health_bar(tank1)->outer);
    scene_add_body(scene, tank_get_health_bar(tank1)->health_pool);

    body_t *tank2 = tank_init(1.0, (rgb_color_t){0.0, 0.0, 0.0}, (vector_t) {80.0, 50.0}, TANK_SIZE, 2);
    scene_add_body(scene,tank2);

    body_t *terrain = generate_terrain(MAX.x, BASE_HEIGHT, TERRAIN_SCALE, NUM_TERRAIN_LEVELS, TERRAIN_DAMPING, TERRAIN_MASS);
    // scene_add_body(scene, terrain);




    sdl_init(MIN, MAX);
    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }
    scene_free(scene);
    return 0;
}
