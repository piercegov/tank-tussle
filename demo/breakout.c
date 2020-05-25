#include "sdl_wrapper.h"
#include "polygon.h"
#include "vector.h"
#include "scene.h"
#include "color.h"
#include "forces.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "body.h"

// Cartesian Coordinates (not pixel values)
const vector_t MAX = {200.0, 100.0};
const int NUM_ROWS = 3;
const int NUM_COLS = 9;
const double CIRC_DENSITY = 20.0;
const double PI = 3.14159265;
const vector_t BALL_VELO = { 1.0, 50.0 };
const vector_t SLIDER_VELO = { 50.0, 0 };
const vector_t PUP_VELO = { 0.0, -20.0 };
const double MASS = 1.0;
const double INTERVAL = 10.0;

const double SLIDER_HEIGHT = 4.0;
const double SLIDER_WIDTH = 16.0;
const double BRICK_HEIGHT = 20.0;
const double BRICK_WIDTH = 4.0;
const double BALL_SIZE = 4.0;
const double WALL_ELASTICITY = 1.0;
const double BRICK_ELASTICITY = 1.0001;
const double BALL_ELASTICITY = 1.0;
const double WALL_WIDTH = 1.0;
const double PUP_SIZE = 4.0;
const double GAP = 2.0;

const rgb_color_t WALL_COLOR = {1.0, 1.0, 1.0};
const vector_t START_POS = { 100.0, 4.0 };
const vector_t BALL_POS = { 100.0, 12.0 };

const rgb_color_t BLACK = {0.0, 0.0, 0.0};
const rgb_color_t RED = {1.0, 0.0, 0.0};
const rgb_color_t TURQUOISE = {64.0 / 256.0, 224.0 / 256.0, 208 / 256.0};
const rgb_color_t RAINBOW[] =
{{1.0, 0.0, 0.0}, {255.0 / 256.0, 165.0 / 265.0, 0.0}, {255.0 / 256.0, 255.0 / 256.0, 0.0},
{0.0, 128.0 / 256.0, 0.0}, {0.0, 0.0, 255.0 / 256.0}, {75.0 / 256.0, 0.0, 130.0 / 256.0},
{238.0 / 256.0, 130.0 / 256.0, 238.0 / 256.0}};

typedef enum {
    BALL,
    SLIDER,
    WALL,
    BRICK,
    LOSER,
    POWER
} body_type_t;

typedef struct power_up_aux {
    scene_t *scene;
} power_up_aux_t;

body_type_t *make_type_info(body_type_t type) {
    body_type_t *info = malloc(sizeof(*info));
    *info = type;
    return info;
}

body_type_t get_type(body_t *body) {
    return *(body_type_t *) body_get_info(body);
}

/** Constructs a rectangle with the given dimensions centered at (0, 0) */
list_t *rect_init(double width, double height) {
    vector_t half_width  = {.x = width / 2, .y = 0.0},
             half_height = {.x = 0.0, .y = height / 2};
    list_t *rect = list_init(4, free);
    vector_t *v = malloc(sizeof(*v));
    *v = vec_add(half_width, half_height);
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_subtract(half_height, half_width);
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_negate(*(vector_t *) list_get(rect, 0));
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_subtract(half_width, half_height);
    list_add(rect, v);
    return rect;
}

// Creates a portion of a circle of diameter d and arclength rads*d at (0,0).
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

void add_slider(scene_t *scene) {
    body_type_t *body_type = malloc(sizeof(int));
    *body_type = SLIDER;
    list_t *slider_pts = rect_init(SLIDER_WIDTH, SLIDER_HEIGHT);
    body_t *slider = body_init_with_info(slider_pts, INFINITY, RED, make_type_info(SLIDER), free);
    body_set_centroid(slider, START_POS);
    body_set_velocity(slider, VEC_ZERO);
    scene_add_body(scene, slider);
}

void add_bricks(scene_t *scene) {
    double y_pos = MAX.y - (MAX.x / NUM_COLS) / 2;
    for (int i = 0; i < NUM_ROWS; i++) {
        double x_pos = (GAP / 2) + ((MAX.x / NUM_COLS) / 2);
        int color_counter = 0;
        for (int j = 0; j < NUM_COLS; j++) {
            int *body_type = malloc(sizeof(int));
            *body_type = BRICK;

            vector_t center = { x_pos, y_pos };
            list_t *brick_pts = rect_init(BRICK_HEIGHT, BRICK_WIDTH);
            body_t *brick = body_init_with_info(brick_pts, INFINITY, RAINBOW[color_counter % 7], make_type_info(BRICK), free);
            body_set_centroid(brick, center);
            color_counter++;
            x_pos += GAP + BRICK_HEIGHT;
            scene_add_body(scene, brick);
        }
        y_pos -= BRICK_WIDTH + GAP;
    }
}

void add_ball(scene_t *scene) {
    list_t *ball_pts = create_arc(BALL_SIZE, 2*PI);
    body_t *ball = body_init_with_info(ball_pts, MASS, BLACK, make_type_info(BALL), free);
    body_set_centroid(ball, BALL_POS);
    body_set_velocity(ball, BALL_VELO);
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        switch (get_type(body)) {
            case BALL:
                create_physics_collision(scene, BALL_ELASTICITY, ball, body);
                break;
            case SLIDER:
                // Bounce off slider
                create_physics_collision(scene, BRICK_ELASTICITY, ball, body);
                break;
            case WALL:
                // Bounce off walls
                create_physics_collision(scene, WALL_ELASTICITY, ball, body);
                break;
            case BRICK:
                // Bounce off bricks and also remove brick
                create_oneway_destructive_collision(scene, ball, body);
                create_physics_collision(scene, BRICK_ELASTICITY, ball, body);
                break;
                // Reset the game
            case LOSER:
                create_oneway_destructive_collision(scene, scene_get_body(scene, i), ball);
                break;
                // Power up
            case POWER:
                break;
        }
    }
    scene_add_body(scene, ball);
}

void render_slider(vector_t velocity, scene_t *scene) {
    body_t *slider = scene_get_body(scene, 0);
    body_set_velocity(slider, velocity);
}

void power_up_handler(body_t *body1, body_t *body2, vector_t axis, power_up_aux_t *aux) {
    scene_t *scene = aux->scene;
    add_ball(scene);
    body_remove(body2);
}

void add_power_up(scene_t *scene) {
     int rand_x = (rand() % (int)(MAX.x - PUP_SIZE)) + PUP_SIZE;
     list_t *points = create_arc(PUP_SIZE, 2*PI);
     body_t *ball = body_init_with_info(points, MASS, TURQUOISE, make_type_info(POWER), free);
     body_set_centroid(ball, (vector_t){ rand_x, MAX.y });
     body_set_velocity(ball, PUP_VELO);
     power_up_aux_t *aux = malloc(sizeof(power_up_aux_t));
     aux->scene = scene;
     create_collision(scene,
         scene_get_body(scene, 0),
         ball,
         (collision_handler_t) power_up_handler,
         aux,
         (free_func_t) free);
         scene_add_body(scene, ball);
}

void slider_key_handler(char key, key_event_type_t type, double held_time, scene_t *scene) {
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                render_slider(vec_multiply(-1, SLIDER_VELO), scene);
                break;

            case RIGHT_ARROW:
                render_slider(SLIDER_VELO, scene);
                break;
        }
    }
    else if (type == KEY_RELEASED) {
        render_slider(VEC_ZERO, scene);
    }
}

/** Adds the walls to the scene */
void add_walls(scene_t *scene) {
    list_t *rect = rect_init(MAX.x, WALL_WIDTH);
    body_t *body = body_init_with_info(
        rect,
        INFINITY,
        WALL_COLOR,
        make_type_info(WALL),
        free
    );
    body_set_centroid(body, (vector_t){MAX.x / 2, MAX.y - WALL_WIDTH / 2});
    scene_add_body(scene, body);

    rect = rect_init(MAX.x, WALL_WIDTH);
    polygon_rotate(rect, PI / 2, VEC_ZERO);
    body = body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(WALL), free);
    body_set_centroid(body, (vector_t){VEC_ZERO.x - WALL_WIDTH / 2, MAX.y / 2});
    scene_add_body(scene, body);

    rect = rect_init(MAX.x, WALL_WIDTH);
    polygon_rotate(rect, PI / 2, VEC_ZERO);
    body = body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(WALL), free);
    body_set_centroid(body, (vector_t){MAX.x + WALL_WIDTH / 2, MAX.y / 2});
    scene_add_body(scene, body);

    // Ground is special; it restarts the game
    rect = rect_init(MAX.x, WALL_WIDTH);
    body = body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(LOSER), free);
    body_set_centroid(body, (vector_t) {MAX.x / 2, -WALL_WIDTH / 2});
    scene_add_body(scene, body);
}

void start_game(scene_t *scene) {
    add_slider(scene);
    add_walls(scene);
    add_bricks(scene);
    add_ball(scene);
}

bool check_ball(scene_t *scene) {
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (get_type(body) == BALL) {
            return true;
        }
    }
    return false;
}

int main(void) {

    sdl_init(VEC_ZERO, MAX);
    scene_t *scene = scene_init();
    double time = 0.0;

    // Add elements to the scene
    start_game(scene);
    sdl_on_key((key_handler_t) slider_key_handler);

    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        if (!check_ball(scene)) {
            scene_free(scene);
            scene = scene_init();
            start_game(scene);
        }
        if (time > INTERVAL) {
            add_power_up(scene);
            time = 0.0;
        }
        time += dt;
        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }

    scene_free(scene);
}


