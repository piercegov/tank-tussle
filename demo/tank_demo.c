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
#include "power_bar.h"
#include "fuel_bar.h"

// Cartesian Coordinates (not pixel values)
const vector_t MIN = {0.0, 0.0};
const vector_t MAX = {200.0, 100.0};

const double PI = 3.14159265;

const double TANK_SIZE = 2.0;
const double SPRITE_SIZE = 30.0;
const double CLOUD_SIZE = 1.0;
const size_t NUM_CLOUDS = 8;
const vector_t CLOUD_SPRITE_SIZE = {128.0, 64.0};
const vector_t TANK_SPRITE_SIZE = {34.0, 26.0};
const vector_t BULLET_SPRITE_SIZE = {16.0, 9.0};
const vector_t TANK_VELO = {100.0, 0};
const vector_t TANK1_START_POS = {20.0, 50.0};
const vector_t TANK2_START_POS = {180.0, 50.0};

const rgb_color_t BLACK = {0.0, 0.0, 0.0};
const double DAMAGE = 25.0; //will need to update how this works with variable bullet types
const double BULLET_MASS = 1.0;
const double BULLET_SIZE = 1.0;
const double G = 690.0;

const double BASE_POWER = 10.0;

const double BASE_HEIGHT = 30.0;
const double TERRAIN_SCALE = 20.0;
const int NUM_TERRAIN_LEVELS = 7;
const double TERRAIN_DAMPING = 0.5;
const double TERRAIN_MASS = 10.0;
const double TERRAIN_AMPLITUDE = 0.8;
const double FUEL_CONSTANT = 2.0;
const double NEW_FUEL = 10.0;
const double WIND = 0.0;
const double WALL_WIDTH = 1.0;

const double BIG_MASS = 100000000.0;
const int ANCHOR_OFF = 100000;
const int ANCHOR_HEIGHT = 10;

const int GAME_LEVELS = 2;
const rgb_color_t LIGHT_BLUE = {173.0 / 255.0, 216.0 / 255.0, 230.0 / 255.0};
const rgb_color_t LIGHT_GRAY = {211.0 / 255.0, 211.0 / 255.0, 211.0 / 255.0};

typedef struct bullet_info {
    double damage;
    double blast_radius;
    //Variable bullet types can be added here
} bullet_info_t;

body_t *tank1;
body_t *tank2;
body_t *terrain;
body_t *anchor;
body_t *left_wall;
body_t *right_wall;
body_t *bottom_wall;

body_t *tank_turn(scene_t *scene, body_t *tank1, body_t *tank2) {
    bool turn1 = tank_get_turn(tank1);
    if (turn1) {
        return tank1;
    }
    return tank2;
}

void add_walls(scene_t *scene) {
    list_t *rect = create_rectangle((vector_t){VEC_ZERO.x - WALL_WIDTH / 2, MAX.y / 2}, WALL_WIDTH, MAX.x);
    left_wall = body_init(rect, INFINITY, BLACK);
    scene_add_body(scene, left_wall);

    rect = create_rectangle((vector_t){MAX.x + WALL_WIDTH / 2, MAX.y / 2}, WALL_WIDTH, MAX.x);
    right_wall = body_init(rect, INFINITY, BLACK);
    scene_add_body(scene, right_wall);

    rect = create_rectangle((vector_t) {MAX.x / 2, -WALL_WIDTH / 2}, WALL_WIDTH, MAX.x);
    polygon_rotate(rect, PI / 2, VEC_ZERO);
    bottom_wall = body_init(rect, INFINITY, BLACK);
    scene_add_body(scene, bottom_wall);
}

void add_texture(body_t *b, SDL_Texture *texture, vector_t sprite_size) {
    body_set_texture(b, texture, sprite_size);
    body_set_texture_rect(b, body_get_centroid(b), sprite_size);
}

void render_tank(body_t *tank, double angle, double power, vector_t velocity, double held_time) {
    double current_angle = tank_get_angle(tank);
    double current_power = tank_get_power(tank);

    tank_set_angle(tank, current_angle+angle);
    tank_set_power(tank, current_power+power);

    if (tank_get_fuel(tank) > 0) {
        body_set_velocity(tank, velocity);
        if (velocity.x != 0) {
            tank_decrease_fuel(tank, FUEL_CONSTANT);
        }
    }
    else if (tank_get_fuel(tank) <= 0) {
        body_set_velocity(tank, VEC_ZERO);
    }
}

void shoot_bullet(scene_t *scene, body_t *tank, double wind) {
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
    SDL_Texture *bullet_text = sdl_create_sprite_texture("images/bullet.png");
    add_texture(bullet, bullet_text, BULLET_SPRITE_SIZE);
    scene_add_body(scene, bullet);
    create_newtonian_gravity(scene, G, bullet, anchor);

    create_bullet_rotate(scene, bullet);
    create_drag(scene, wind, bullet);

    body_t *other_tank;
    if (type == 1) {
        other_tank = tank2;
    }
    else {
        other_tank = tank1;
    }
    create_damaging_collision(scene, other_tank, bullet);
    double current_fuel = tank_get_fuel(other_tank);
    if (current_fuel + NEW_FUEL > 100) {
        tank_set_fuel(other_tank, 100.0);
    }
    else {
        tank_set_fuel(other_tank, current_fuel + NEW_FUEL);
    }
    tank_set_turn(other_tank, true);
    update_fuel_bar(other_tank);
    create_bullet_destroy(scene, terrain, bullet);
    tank_set_turn(tank, false);
}

bool off_screen_right(body_t *tank) {
    list_t *shape = body_get_shape(tank);
    for (size_t i = 0; i < list_size(shape); i++) {
        vector_t *current_vec = list_get(shape, i);
        if (current_vec->x >= MAX.x - (TANK_SIZE)) {
            return true;
        }
    }
    return false;
}

bool off_screen_left(body_t *tank) {
    list_t *shape = body_get_shape(tank);
    for (size_t i = 0; i < list_size(shape); i++) {
        vector_t *current_vec = list_get(shape, i);
        if (current_vec->x <= MIN.x + (TANK_SIZE)) {
            return true;
        }
    }
    return false;
}

void shooter_key_handler(char key, key_event_type_t type, double held_time, scene_t *scene) {
    body_t *tank = tank_turn(scene, tank1, tank2);
    // body_t *last = scene_get_body(scene, (scene_bodies(scene) - 1);
    // if (body_get_info) {
    //
    // }
    // else {
        if (type == KEY_PRESSED) {
            switch (key) {
                case LEFT_ARROW:
                    if (off_screen_left(tank)) {
                        render_tank(tank, 0.0, 0.0, (vector_t){ 0 , 0 }, held_time);
                    }
                    else {
                        render_tank(tank, 0.0, 0.0, (vector_t) {-TANK_VELO.x, TANK_VELO.y}, held_time);
                    }
                    break;

                case RIGHT_ARROW:
                    if (off_screen_right(tank)) {
                        render_tank(tank, 0.0, 0.0, (vector_t){ 0 , 0 }, held_time);
                    }
                    else {
                        render_tank(tank, 0.0, 0.0, (vector_t) {TANK_VELO.x, TANK_VELO.y}, held_time);
                    }
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
                    shoot_bullet(scene, tank, WIND);
            }
        }

        else if (type == KEY_RELEASED) {
            render_tank(tank, 0.0, 0.0, (vector_t){ 0 , 0 }, held_time);
        }
    // }
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

    vector_t *p1 = vec_init(MIN.x, MIN.y - (ANCHOR_OFF + ANCHOR_HEIGHT));
    list_add(list, p1);

    vector_t *p2 = vec_init(MAX.x, MIN.y - (ANCHOR_OFF + ANCHOR_HEIGHT));
    list_add(list, p2);

    vector_t *p3 = vec_init(MAX.x, MIN.y - ANCHOR_OFF);
    list_add(list, p3);

    rgb_color_t color = { 1, 1, 1 };
    return body_init(list, BIG_MASS, color);
}

void make_clouds(scene_t *scene) {
    for (size_t i = 0; i < NUM_CLOUDS; i++) {
        list_t *pts = create_arc(2 * PI, CLOUD_SIZE);
        body_t *b = body_init(pts, 1, BLACK);
        body_set_centroid(b, (vector_t){ 20 * (i), (double)(rand() % ((int)MAX.y / 2)) + (double)(MAX.y / 2) });
        SDL_Texture *texture = sdl_create_sprite_texture("images/cloud.png");
        add_texture(b, texture, CLOUD_SPRITE_SIZE);
        scene_add_body(scene, b);
    }
}

scene_t *init_new_game(int rand_num) {
    scene_t *scene = scene_init();
    body_t *sky;

    if (rand_num > 4) {
        sky = make_sky(LIGHT_GRAY);
    }
    else {
        sky = make_sky(LIGHT_BLUE);
    }
    scene_add_body(scene, sky);

    terrain = generate_terrain(MAX.x, BASE_HEIGHT, TERRAIN_SCALE, NUM_TERRAIN_LEVELS, TERRAIN_DAMPING, TERRAIN_MASS, TERRAIN_AMPLITUDE);
    scene_add_body(scene, terrain);

    SDL_Texture *texture1 = sdl_create_sprite_texture("images/tank_big_blue.png"); //need to import image
    SDL_Texture *texture2 = sdl_create_sprite_texture("images/tank_big_red.png"); //need to import image

    tank1 = tank_init(1.0, (rgb_color_t){0.0, 0.0, 0.0}, TANK1_START_POS, TANK_SIZE, 1);
    scene_add_body(scene,tank1);
    tank2 = tank_init(1.0, (rgb_color_t){0.0, 0.5, 0.5}, TANK2_START_POS, TANK_SIZE, 2);
    scene_add_body(scene,tank2);

    add_texture(tank1, texture1, TANK_SPRITE_SIZE);
    add_texture(tank2, texture2, TANK_SPRITE_SIZE);
    create_terrain_follow(scene, terrain, tank1);
    create_terrain_follow(scene, terrain, tank2);

    make_clouds(scene);

    add_walls(scene);
    create_physics_collision(scene, 0.001, tank1, left_wall);
    create_physics_collision(scene, 0.001, tank1, right_wall);
    // // create_oneway_destructive_collision(scene, scene_get_body(scene, 6), tank1);
    create_physics_collision(scene, 0.001, tank2, left_wall);
    create_physics_collision(scene, 0.001, tank2, right_wall);
    // create_oneway_destructive_collision(scene, scene_get_body(scene, 6), tank2);

    anchor = make_ground();
    scene_add_body(scene, anchor);

    health_bar_t *health_bar1 = tank_get_health_bar(tank1);
    scene_add_body(scene, health_bar1->outer);
    scene_add_body(scene, health_bar1->inner);
    scene_add_body(scene, health_bar1->health_pool);
    health_bar_t *health_bar2 = tank_get_health_bar(tank2);
    scene_add_body(scene, health_bar2->outer);
    scene_add_body(scene, health_bar2->inner);
    scene_add_body(scene, health_bar2->health_pool);
    create_health_follow(scene, tank1);
    create_health_follow(scene, tank2);

    power_bar_t *power_bar1 = tank_get_power_bar(tank1);
    scene_add_body(scene, power_bar1->outer);
    scene_add_body(scene, power_bar1->inner);
    scene_add_body(scene, power_bar1->power_level);
    power_bar_t *power_bar2 = tank_get_power_bar(tank2);
    scene_add_body(scene, power_bar2->outer);
    scene_add_body(scene, power_bar2->inner);
    scene_add_body(scene, power_bar2->power_level);

    fuel_bar_t *fuel_bar1 = tank_get_fuel_bar(tank1);
    scene_add_body(scene, fuel_bar1->outer);
    scene_add_body(scene, fuel_bar1->inner);
    scene_add_body(scene, fuel_bar1->fuel_level);
    fuel_bar_t *fuel_bar2 = tank_get_fuel_bar(tank2);
    scene_add_body(scene, fuel_bar2->outer);
    scene_add_body(scene, fuel_bar2->inner);
    scene_add_body(scene, fuel_bar2->fuel_level);

    return scene;
}

int main() {
    srand(time(0));
    sdl_init(MIN, MAX);
    scene_t *scene = init_new_game(rand() % 10);

    sdl_on_key((key_handler_t) shooter_key_handler);

    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        scene_tick(scene, dt);
        sdl_render_scene(scene);

        int tank1_wins = 0;
        int tank2_wins = 0;

        if (tank_is_dead(tank1)) {
            printf("Player 2 wins!\n"); //Text rendering later?
            tank2_wins++;
            if (tank2_wins < GAME_LEVELS) {
                scene_free(scene);
                scene = init_new_game(rand() % 10);
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
                scene = init_new_game(rand() % 10);
            }
            else {
                printf("Game over, Player 1 has won!\n");
            }
        }
    }
    scene_free(scene);
    return 0;
}
