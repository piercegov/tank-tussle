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
#include "forces.h"
#include "math.h"

const vector_t MIN = {0.0, 0.0};
const double MASS = 1.0;
const double ANCHOR_MASS = INFINITY;
const vector_t MAX = {200.0, 100.0};
const double CIRC_DENSITY = 10.0;
const double PI = 3.14159265;
const double K = 5;
const double GAMMA = 0.1;
const int NUM_BODIES = 60;

body_t *create_arc(double d, double rads, rgb_color_t color, double mass) {
    double total_pts = CIRC_DENSITY * rads;
    list_t *points = list_init(total_pts, (free_func_t) free);
    for (size_t i = 0; i < total_pts; i++) {
        vector_t *new_pt = malloc(sizeof(vector_t));
        *new_pt = vec_rotate((vector_t) {d / 2, 0}, i / CIRC_DENSITY);
        list_add(points, new_pt);
    }
    return body_init(points, mass, color);
}

void add_bodies(scene_t *scene) {
    double x_pos = 0;
    double y_pos = MAX.y;
    double angle = 0;
    for (size_t i = 0; i < NUM_BODIES; i++) {
        x_pos += MAX.x / NUM_BODIES;
        vector_t anchor_pos = { x_pos, MAX.y / 2 };
        body_t *anchor = create_arc(MAX.x / NUM_BODIES, 2 * PI, (rgb_color_t) {1, 1, 1}, ANCHOR_MASS);
        body_set_centroid(anchor, anchor_pos);
        scene_add_body(scene, anchor);
    }
    x_pos = 0;
    for (size_t i = 0; i < NUM_BODIES; i++) {
        x_pos += MAX.x / NUM_BODIES;
        vector_t pos = { x_pos, y_pos };
        rgb_color_t color = { (float)rand() / (float)RAND_MAX, (float)rand() /
                    (float)RAND_MAX, (float) rand() / (float)RAND_MAX };
        body_t *circle = create_arc(MAX.x / NUM_BODIES, 2 * PI, color, MASS);
        body_set_centroid(circle, pos);
        scene_add_body(scene, circle);
        y_pos -= (MAX.y * sin(angle)) / NUM_BODIES;
        angle += PI / (NUM_BODIES * 2);
    }
}

void apply_forces(scene_t *scene) {
    size_t body_count = scene_bodies(scene) / 2;
    for (size_t i = 0; i < body_count; i++) {
        create_spring(scene, K, scene_get_body(scene, i), scene_get_body(scene, i + body_count));
        create_drag(scene, GAMMA, scene_get_body(scene, i + body_count));
    }
}

int main() {
    srand(time(NULL));
    scene_t *scene = scene_init();
    add_bodies(scene);
    apply_forces(scene);
    sdl_init(MIN, MAX);
    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }
    scene_free(scene);
    return 0;
}
