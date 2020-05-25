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

const vector_t MIN = {0.0, 0.0};
const vector_t MAX = {200.0, 100.0};
const int STAR_SIDES = 8;
const int MAX_STAR_SIZE = 10;
const int NUM_STARS = 100;
const double G = 0.1;
const double PI = 3.14159;
const double MASS_MULT = 100.0;

body_t *create_star(double size, vector_t center, int num_sides) {
    list_t *points = list_init(num_sides,  (free_func_t) free);
    for (int i = 0; i < num_sides; i++) {
        vector_t *next_pt = malloc(sizeof(vector_t));
        assert(next_pt != NULL);
        double angle = ((double) i / (double)num_sides) * (2 * PI);
        double rad = 0.0;

        if (i % 2 == 0) {
            rad = size;
        } else {
            rad = size / 2;
        }

        *next_pt = vec_add((vector_t) { rad * cos(angle),
                        rad * sin(angle) }, center);
        list_add(points, next_pt);
    }
    rgb_color_t color = { (float)rand() / (float)RAND_MAX,
        (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX };
    return body_init(points, MASS_MULT * size, color);
}

void add_stars(scene_t *scene) {
    for (int i = 0; i < NUM_STARS; i++) {
        int size = rand() % MAX_STAR_SIZE + 4;
        double rand_x = (rand() % ((int)MAX.x - (2 * size))) + (size);
        double rand_y = (rand() % ((int)MAX.y - (2 * size))) + (size);
        body_t *body = create_star(size, (vector_t){ rand_x, rand_y }, STAR_SIDES);
        scene_add_body(scene, body);
    }
}

void apply_gravity(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        for (size_t j = i + 1; j < scene_bodies(scene); j++) {
            create_newtonian_gravity(scene, G, scene_get_body(scene, i), scene_get_body(scene, j));
        }
    }
}

int main() {
    scene_t *scene = scene_init();
    srand(time(NULL));
    add_stars(scene);
    sdl_init(MIN, MAX);
    apply_gravity(scene);
    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        scene_tick(scene, dt);
        sdl_render_scene(scene);
    }
    scene_free(scene);
    return 0;
}
