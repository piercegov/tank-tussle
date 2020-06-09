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
#include "bullet_types.h"
#include "fuel_bar.h"
#include <unistd.h>

// Cartesian Coordinates (not pixel values)
const vector_t MIN = {0.0, 0.0};
const vector_t MAX = {200.0, 100.0};

const double TANK_SIZE = 2.0;
const double SPRITE_SIZE = 30.0;
const double CLOUD_SIZE = 1.0;
const size_t NUM_CLOUDS = 8;
const vector_t CLOUD_SPRITE_SIZE = {128.0, 64.0};
const vector_t TANK_SPRITE_SIZE = {34.0, 26.0};
const vector_t BARREL_SIZE = {30.0, 10.0};
const vector_t TANK_VELO = {20.0, 0};
const vector_t TANK1_START_POS = {20.0, 50.0};
const vector_t TANK2_START_POS = {180.0, 50.0};

const double DAMAGE = 25.0; //will need to update how this works with variable bullet types
const double BULLET_MASS = 1.0;
const double BASE_POWER = 30.0;

const double BASE_HEIGHT = 30.0;
const double TERRAIN_SCALE = 20.0;
const int NUM_TERRAIN_LEVELS = 7;
const double TERRAIN_DAMPING = 0.5;
const double TERRAIN_MASS = 10.0;
const double TERRAIN_AMPLITUDE = 0.8;

const double WIND = 0.0;
const double WALL_WIDTH = 1.0;
const vector_t TEXT_SIZE = { 150.0, 50.0 };

const double BIG_MASS = 100000000.0;
const double TANK_MASS = 1000.0;
const int ANCHOR_OFF = 100000;
const int ANCHOR_HEIGHT = 10;
const vector_t BARREL_DIM = {4.0, 0.00000001};

const int GAME_LEVELS = 2;
const rgb_color_t LIGHT_BLUE = {173.0 / 255.0, 216.0 / 255.0, 230.0 / 255.0};
const rgb_color_t LIGHT_GRAY = {211.0 / 255.0, 211.0 / 255.0, 211.0 / 255.0};
const rgb_color_t TERRAIN_GREEN = {0.0, 153.0/255.0, 51.0/255.0};

body_t *tank1;
body_t *tank2;
body_t *terrain;
body_t *anchor;
body_t *left_wall;
body_t *right_wall;

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
    scene_walls(scene, left_wall, right_wall);
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

void shoot_bullet(scene_t *scene, double wind, double dmg) {
    vector_t tank_center;
    body_t *t1;
    body_t *t2;
    if (tank_get_turn(tank1)) {
        t1 = tank1;
        t2 = tank2;
        tank_center = body_get_centroid(tank_get_barrel(tank1));
    } else {
        t2 = tank1;
        t1 = tank2;
        tank_center = body_get_centroid(tank_get_barrel(tank2));
    }

    double angle = tank_get_angle(t1);
    angle = (angle * PI) / 180.0;
    double x_dir = cos(angle);
    double y_dir = sin(angle);

    double power = tank_get_power(t1);
    power = power + BASE_POWER;
    vector_t velo = vec_multiply(power, (vector_t) {x_dir, y_dir});

    int type = tank_get_number(t1);
    if (type == 2) {
        velo.x = velo.x * -1;
    }
    // create_cluster_bomb(scene, t2, t1, tank_center, velo, wind, dmg);
    create_kinetic_bullet(scene, t2, t1, tank_center, velo, wind, dmg);
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
    body_t *last = scene_get_body(scene, (scene_bodies(scene) - 1));
    if (body_get_type(last) == 0) {
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
                    if (tank_get_angle(tank) < -45) {
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
                    shoot_bullet(scene, WIND, DAMAGE);
            }
        }

        else if (type == KEY_RELEASED) {
            render_tank(tank, 0.0, 0.0, (vector_t){ 0 , 0 }, held_time);
        }
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
        body_set_centroid(b, (vector_t){ 25 * (i), (double)(rand() % ((int)MAX.y / 2)) + (double)(MAX.y / 2) });
        SDL_Texture *texture = sdl_create_sprite_texture("images/cloud.png");
        add_texture(b, texture, CLOUD_SPRITE_SIZE);
        scene_add_body(scene, b);
    }
}

void add_text_bars(scene_t *scene, vector_t center, vector_t dimensions, rgb_color_t color, char message[]) {
    list_t *rect = create_rectangle(center, 1.0, 1.0);
    body_t *b = body_init(rect, 1.0, color);
    body_set_centroid(b, center);
    SDL_Texture *mess = sdl_create_text(message, "fonts/Lordcorps.ttf", 150);
    add_texture(b, mess, vec_multiply(6, dimensions));
    scene_add_body(scene, b);
}

scene_t *init_new_game(rgb_color_t sky_color) {
    scene_t *scene = scene_init();
    body_t *sky = make_sky(sky_color);
    scene_add_body(scene, sky);

    tank1 = tank_init(TANK_MASS, (rgb_color_t){0.0, 0.0, 0.0}, TANK1_START_POS, TANK_SIZE, 1, BARREL_DIM);
    tank2 = tank_init(TANK_MASS, (rgb_color_t){0.0, 0.5, 0.5}, TANK2_START_POS, TANK_SIZE, 2, BARREL_DIM);
    body_set_color(tank_get_barrel(tank1), sky_color);
    body_set_color(tank_get_barrel(tank2), sky_color);
    scene_add_body(scene, tank_get_barrel(tank1));
    scene_add_body(scene, tank_get_barrel(tank2));

    terrain = generate_terrain(MAX.x, BASE_HEIGHT, TERRAIN_SCALE, NUM_TERRAIN_LEVELS, TERRAIN_DAMPING, TERRAIN_MASS, TERRAIN_AMPLITUDE);
    scene_add_body(scene, terrain);
    scene_terrain(scene, terrain);

    SDL_Texture *texture1 = sdl_create_sprite_texture("images/tank_big_blue.png");
    SDL_Texture *texture2 = sdl_create_sprite_texture("images/tank_big_red.png");

    SDL_Texture *barrel1 = sdl_create_sprite_texture("images/barrel_blue.png");
    SDL_Texture *barrel2 = sdl_create_sprite_texture("images/barrel_blue.png");

    scene_add_body(scene, tank1);
    scene_add_body(scene, tank2);
    add_texture(tank_get_barrel(tank1), barrel1, BARREL_SIZE);
    add_texture(tank_get_barrel(tank2), barrel2, BARREL_SIZE);
    create_barrel_rotate(scene, tank1, BARREL_DIM);
    create_barrel_rotate(scene, tank2, vec_negate(BARREL_DIM));

    scene_tanks(scene, tank1, tank2);

    // Add textures to tanks and barrels
    add_texture(tank1, texture1, TANK_SPRITE_SIZE);
    add_texture(tank2, texture2, TANK_SPRITE_SIZE);

    // Make tanks follow the terrain level
    create_terrain_follow(scene, terrain, tank1);
    create_terrain_follow(scene, terrain, tank2);

    make_clouds(scene);

    add_walls(scene);
    create_physics_collision(scene, 0.001, tank1, left_wall);
    create_physics_collision(scene, 0.001, tank1, right_wall);
    create_physics_collision(scene, 0.001, tank2, left_wall);
    create_physics_collision(scene, 0.001, tank2, right_wall);

    anchor = make_ground();
    scene_add_body(scene, anchor);
    scene_anchor(scene, anchor);

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

    add_text_bars(scene, vec_add((vector_t) {0.0, BAR_HEIGHT}, body_get_centroid(power_bar1->outer)), (vector_t){ BAR_WIDTH, BAR_HEIGHT }, TERRAIN_GREEN, "PLAYER 1 POWER");
    add_text_bars(scene, vec_add((vector_t) {0.0, BAR_HEIGHT}, body_get_centroid(power_bar2->outer)), (vector_t){ BAR_WIDTH, BAR_HEIGHT }, TERRAIN_GREEN, "PLAYER 2 POWER");

    fuel_bar_t *fuel_bar1 = tank_get_fuel_bar(tank1);
    scene_add_body(scene, fuel_bar1->outer);
    scene_add_body(scene, fuel_bar1->inner);
    scene_add_body(scene, fuel_bar1->fuel_level);
    fuel_bar_t *fuel_bar2 = tank_get_fuel_bar(tank2);
    scene_add_body(scene, fuel_bar2->outer);
    scene_add_body(scene, fuel_bar2->inner);
    scene_add_body(scene, fuel_bar2->fuel_level);
    add_text_bars(scene, vec_add((vector_t) {0.0, BAR_HEIGHT}, body_get_centroid(fuel_bar1->outer)), (vector_t){ BAR_WIDTH, BAR_HEIGHT }, TERRAIN_GREEN, "PLAYER 1 FUEL");
    add_text_bars(scene, vec_add((vector_t) {0.0, BAR_HEIGHT}, body_get_centroid(fuel_bar2->outer)), (vector_t){ BAR_WIDTH, BAR_HEIGHT }, TERRAIN_GREEN, "PLAYER 2 FUEL");

    add_text_bars(scene, (vector_t) {MAX.x / 2, 7 * MAX.y / 8}, (vector_t) {150.0, 4.0}, sky_color,
        "Move: Left/right || Power: w/s || Angle: Up/down || Shoot: Space bar || Bullet Type: 1 (Normal), 2 (Scatter), 3 (Radius)");

    return scene;
}

rgb_color_t make_sky_color() {
    rgb_color_t sky_color;
    int rand_num = rand() % 10;
    if (rand_num > 4) {
        sky_color = LIGHT_GRAY;
    }
    else {
        sky_color = LIGHT_BLUE;
    }
    return sky_color;
}

int main() {
    srand(time(0));
    sdl_init(MIN, MAX);
    rgb_color_t sky_color = make_sky_color();

    scene_t *scene = init_new_game(sky_color);

    sdl_on_key((key_handler_t) shooter_key_handler);

    int tank1_wins = 0;
    int tank2_wins = 0;

    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        scene_tick(scene, dt);
        sdl_render_scene(scene);

        if (tank_is_dead(tank1)) {
            tank2_wins++;
            if (tank2_wins < GAME_LEVELS) {
                add_text_bars(scene, (vector_t) {MAX.x / 2, 5 * MAX.y / 8}, (vector_t) { 50.0, 4.0 }, sky_color, "PLAYER 2 WINS THE ROUND!!!");
                sdl_render_scene(scene);
                sleep(5);
                scene_free(scene);
                sky_color = make_sky_color();
                scene = init_new_game(sky_color);
            }
            else {
                add_text_bars(scene, (vector_t) {MAX.x / 2, 5 * MAX.y / 8}, (vector_t) { 100.0, 4.0 }, sky_color, "Game over, Player 2 has won!!!");
                sdl_render_scene(scene);
                sleep(5);
                break;
            }
        }

        else if (tank_is_dead(tank2)) {
            tank1_wins++;
            if (tank1_wins < GAME_LEVELS) {
                add_text_bars(scene, (vector_t) {MAX.x / 2, 5 * MAX.y / 8}, (vector_t) { 50.0, 4.0 }, sky_color, "PLAYER 1 WINS THE ROUND!!!");
                sdl_render_scene(scene);
                sleep(5);
                scene_free(scene);
                sky_color = make_sky_color();
                scene = init_new_game(sky_color);
            }
            else {
                add_text_bars(scene, (vector_t) {MAX.x / 2, 5 * MAX.y / 8}, (vector_t) { 100.0, 4.0 }, sky_color, "Game over, Player 1 has won!!!");
                sdl_render_scene(scene);
                sleep(5);
                break;
            }
        }
    }
    scene_free(scene);
    return 0;
}
