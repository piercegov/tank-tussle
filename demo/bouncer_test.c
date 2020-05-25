#include "sdl_wrapper.h"
#include "polygon.h"
#include "vector.h"
#include "scene.h"
#include "color.h"
#include "forces.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "body.h"

// Cartesian Coordinates (not pixel values)
const vector_t MIN = {0.0, 0.0};
const vector_t MAX = {200.0, 100.0};

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

body_t *make_rect(double width, double height, vector_t center, rgb_color_t color){
    list_t *points = rect_init(width, height);
    body_t *body = body_init(points, 10, color);
    body_set_centroid(body, center);
    return body;
}

int main(int argc, char* argv[]) {
    scene_t *scene = scene_init();

    body_t *r1 = make_rect(20, 20, (vector_t) {50, 50}, (rgb_color_t){1, 0, 0});
    body_t *r2 = make_rect(20, 20, (vector_t) {150, 50}, (rgb_color_t){0, 0, 1});
    body_set_velocity(r1, (vector_t){80, 0});

    scene_add_body(scene, r1);
    // body_set_rotation(r1, 40.0);
    scene_add_body(scene, r2);

    create_physics_collision(scene, r1, r2, 1.0);

    sdl_init(VEC_ZERO, MAX);
    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }
    scene_free(scene);
}
