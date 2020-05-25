#include "sdl_wrapper.h"
#include "polygon.h"
#include "vector.h"
#include "scene.h"
#include "color.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "body.h"

// Cartesian Coordinates (not pixel values)
const vector_t MIN = {0.0, 0.0};
const vector_t MAX = {200.0, 100.0};

const double WRAP_OFFSET = 5.0;

const double CIRC_DENSITY = 10.0;
const double PI = 3.14159265;
const vector_t START_POS = {50.0, 50.0};
const vector_t INIT_VELO = {200.0, 200.0};

const rgb_color_t YELLOW = {(float)1.0, (float)1.0, (float)0.0};
const double INTERVAL = 0.1;
const double PELLET_SIZE = 5.0;
const double PACMAN_SIZE = 30.0;

// Creates a portion of a circle of diameter d and arclength rads*d at (0,0).
body_t *create_arc(double d, double rads) {
    double total_pts = CIRC_DENSITY * rads;
    list_t *points = list_init(total_pts, (free_func_t) free);
    for (size_t i = 0; i < total_pts; i++) {
        vector_t *new_pt = malloc(sizeof(vector_t));
        *new_pt = vec_rotate((vector_t) {d / 2, 0}, i / CIRC_DENSITY);
        list_add(points, new_pt);
    }
    return body_init(points, 0.0, YELLOW);
}

void add_pellet(scene_t *scene) {
    vector_t pos = {rand() % (int) MAX.x, rand() % (int) MAX.y};
    body_t *pellet = create_arc(PELLET_SIZE, 2 * PI);
    body_set_centroid(pellet, pos);
    scene_add_body(scene, pellet);
}

void add_pacman(scene_t *scene) {
    body_t *pacman = create_arc(PACMAN_SIZE, 3*PI/2);
    body_set_centroid(pacman, START_POS);
    list_t *pac_points = body_get_points(pacman);
    vector_t *center = malloc(sizeof(vector_t));
    assert(center != NULL);

    *center = body_get_centroid(pacman);
    list_add(pac_points, (void *) center);
    polygon_rotate(pac_points, PI/4, *center);
    scene_add_body(scene, pacman);
}

void off_screen(body_t *pacman) {
    list_t *shapes = body_get_points(pacman);
    size_t size = list_size(shapes);
    vector_t center = body_get_centroid(pacman);
    bool off_x = true;
    bool off_y = true;
    for (size_t i = 0; i < size; i++) {
        vector_t point = *(vector_t *)list_get(shapes, i);
        if (point.x > MIN.x && point.x < MAX.x) {
            off_x = false;
        }
        if (point.y > MIN.y && point.y < MAX.y) {
            off_y = false;
        }
    }
    if (off_x) {
        if (center.x > MAX.x) {
            center.x = MIN.x - (PACMAN_SIZE / 2) + WRAP_OFFSET;
        }
        else {
            center.x = MAX.x + (PACMAN_SIZE / 2) - WRAP_OFFSET;
        }
        body_set_centroid(pacman, center);
    }
    if (off_y) {
        if (center.y > MAX.y) {
            center.y = MIN.y - (PACMAN_SIZE / 2) + WRAP_OFFSET;
        }
        else {
            center.y = MAX.y + (PACMAN_SIZE / 2) - WRAP_OFFSET;
        }
        body_set_centroid(pacman, center);
    }
}

void render_pacman(double angle, vector_t velocity, double held_time, scene_t *scene) {
    body_t *pacman = scene_get_body(scene, 0);
    vector_t new_velocity = vec_add(velocity, vec_multiply(held_time, velocity));
    body_set_velocity(pacman, new_velocity);
    body_set_rotation(pacman, angle);
}

void pacman_key_handler(char key, key_event_type_t type, double held_time, scene_t *scene) {
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                render_pacman(PI, (vector_t){ -INIT_VELO.x, 0 }, held_time, scene);
                break;

            case UP_ARROW:
                render_pacman(PI/2, (vector_t){ 0, INIT_VELO.y }, held_time, scene);
                break;

            case RIGHT_ARROW:
                render_pacman(0, (vector_t) { INIT_VELO.x, 0 }, held_time, scene);
                break;

            case DOWN_ARROW:
                render_pacman(3*PI/2, (vector_t) { 0, -INIT_VELO.y }, held_time, scene);
                break;
        }
    }
    else if (type == KEY_RELEASED) {
        double old_angle = body_get_rotation(scene_get_body(scene, 0));
        vector_t new_velo = vec_rotate((vector_t) { INIT_VELO.x, 0.0 }, old_angle);
        render_pacman(old_angle, new_velo, held_time, scene);
    }
}

void pacman_tick(double dt, scene_t *scene) {
    scene_tick(scene, dt);
    // check collisions
    size_t num_bodies = scene_bodies(scene);
    if (num_bodies <= 1) {
        return;
    }
    body_t *pacman = scene_get_body(scene, 0);
    for (size_t i = num_bodies - 1; i > 0; i--) {
        if (body_is_colliding(pacman, scene_get_body(scene, i))) {
            scene_remove_body(scene, i);
        }
    }
    off_screen(pacman);
}


int main() {
    scene_t *scene = scene_init();
    srand(time(NULL));

    double time_since_last_add = 0.0;
    sdl_init(MIN, MAX);
    add_pacman(scene);
    sdl_on_key((key_handler_t) pacman_key_handler);
    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        pacman_tick(dt, scene);
        time_since_last_add += dt;
        if (time_since_last_add > INTERVAL) {
            add_pellet(scene);
            time_since_last_add = 0.0;
        }
        sdl_render_scene(scene);
    }
    scene_free(scene);
    return 0;
}
