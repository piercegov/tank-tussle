#include "sdl_wrapper.h"
#include "polygon.h"
#include "vector.h"
#include "scene.h"
#include "color.h"
#include "forces.h"
#include "body.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>

const double PI = 3.14159265;
const double CIRC_DENSITY = 40.0;

const double TANK_SIZE = 20.0;
const vector_t TANK_VELO = {100.0, 0};

const rgb_color_t BLACK = {0.0, 0.0, 0.0};
const double DAMAGE = 25.0; //will need to update how this works with variable bullet types
const double BULLET_MASS = 1.0;
const double BULLET_SIZE = 2.0;
const double G = 0.1

const double BASE_POWER = 10.0;

list_t *create_arc(double d, double rads) {
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
    list_t *points = create_arc(BULLET_SIZE, 2*PI);
    body_t bullet = body_init_with_info(points, BULLET_MASS, BLACK, DAMAGE, free);
    vector_t tank_center = tank_get_centroid(tank);
    body_set_centroid(bullet, tank_center);

    double angle = tank_get_angle(tank);
    angle = (angle * PI) / 180.0;
    double x_dir = cos(angle);
    double y_dir = sin(angle);

    double power = tank_get_power(tank);
    vector_t velo = vec_multiply((vector_t) {x_dir, y_dir}, power);
    velo = vec_multiply(velo, BASE_POWER);

    int type = get_tank_type(tank);
    if (type == 1) {
        body_set_velocity(bullet, velo);
    }

    else {
        //If the tank is on the righthand side then the x-velo needs to be flipped
        velo.x = velo.x * -1;
        body_set_velocity(bullet, velo);
    }

    scene_add_body(scene, bullet);
    create_newtonian_gravity(scene, G, tank, scene_get_body(scene, 4));

    if (type == 1) {
        //Second body in scene is tank 2
        body_t tank2 = scene_get_body(scene, 1);
        create_damaging_collision(scene, tank2);
        tank_set_info(tank, false);
        tank_set_info(tank2, true);
    }
    else {
        //Second body in scene is tank 1
        body_t tank1 = scene_get_body(scene, 0);
        create_damaging_collision(scene, tank1);
        tank_set_info(tank, false);
        tank_set_info(tank1, true);
    }
}

tank_t tank_turn(scene) {
    tank_t *tank1 = scene_get_body(scene, 0);
    boolean turn1 = *tank_get_info(tank1);
    if (turn1) {
        return tank1;
    }
    tank_t *tank2 = scene_get_body(scene, 1);
    return tank2;
}

void render_tank(tank_t tank, double angle, double power, vector_t velocity) {
    double current_angle = tank_get_angle(tank);
    double current_power = tank_get_power(tank);
    double current_velo = tank_get_velocity(tank);

    if (get_tank_type(tank) == 1) {
        vector_t new_velocity = vec_add(velocity, vec_multiply(held_time, velocity));
    }

    else {
        vector_t new_velocity = vec_subtract(velocity, vec_multiply(held_time, velocity));
    }

    tank_set_angle(tank, current_angle+angle);
    tank_set_power(tank, current_power+power);
    tank_set_velocity(tank, new_velocity);
}

void shooter_key_handler(char key, key_event_type_t type, double held_time, scene_t *scene) {
    tank_t tank = tank_turn(scene);
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                if (get_tank_type(tank) == 1) {
                    render_tank(tank, 0.0, 0.0, (vector_t) {-TANK_VELO.x, TANK_VELO.y});
                }
                else {
                    render_tank(tank, 0.0, 0.0, TANK_VELO);
                }
                break;

            case RIGHT_ARROW:
                if (get_tank_type(tank) == 2) {
                    render_tank(tank, 0.0, 0.0, (vector_t) {-TANK_VELO.x, TANK_VELO.y});
                }
                else {
                    render_tank(tank, 0.0, 0.0, TANK_VELO);
                }
                break;

            case UP_ARROW:
                render_tank(tank, 1.0, 0.0, (vector_t){ 0 , 0 });
                break;

            case DOWN_ARROW:
                render_tank(tank, -1.0, 0.0, (vector_t){ 0 , 0 });
                break;

            case W:
                render_tank(tank, 0.0, 1.0, (vector_t){ 0 , 0 });
                break;

            case S:
                render_tank(tank, 0.0, -1.0, (vector_t){ 0 , 0 });

            case SPACE_BAR:
                shoot_bullet(scene, tank);
        }
    }

    else if (type == KEY_RELEASED) {
        render_shooter(tank, 0.0, 0.0, (vector_t){ 0 , 0 });
    }
}

void tangent_bodies(body_t *tank, body_t *terrain) {
    vector_t tank_center = body_get_centroid(tank);
    vector_t terrain_max = body_get_max(terrain, tank_center - 1, tank_center + 1);
    body_set_centroid(tank, (vector_t){ tank_center.x, terrain_max.y + TANK_SIZE });
}

int main() {
    scene_t *scene = scene_init();
}
