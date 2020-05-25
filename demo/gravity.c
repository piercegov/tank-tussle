#include "sdl_wrapper.h"
#include "vector.h"
#include "list.h"
#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "math.h"
#include <assert.h>

const vector_t MIN = { 0.0, 0.0 };
const vector_t MAX = { 200.0, 100.0 };
const vector_t INIT_VELO = { 69.0, 0.0 };
const int INIT_PTS = 4;
const int ANCHOR_OFF = 100000;
const int ANCHOR_HEIGHT = 10;
const double ANGLE = 0.01;
const int INIT_STARS = 10;
const int MAX_POINTS = 20;
const int STAR_SIZE = 10;
const double ELASTICITY = .7;
const double TIME_BETWEEN_STARS = 1;
const double PI = 3.14159265;
const double G = 10.0;
const double MASS = 1.0;
const double BIG_MASS = 1000000000000.0;

body_t *make_star(double size, vector_t center, int num_sides) {
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
    return body_init(points, MASS, color);
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

void on_ground(body_t *body, double elasticity) {
    list_t *shape = body_get_shape(body);
    vector_t velo = body_get_velocity(body);
    int on_ground = 0; /* At bottom bool */
    double y_velo = velo.y;
    for (size_t i = 0; i < list_size(shape); i++) {
        /* Check if the star has collided with bottom of the screen or ground */
        vector_t *current_vec = list_get(shape, i);
        if (current_vec->y <= VEC_ZERO.y) {
            on_ground = 1;
        }
    }
    if (on_ground == 1) {
        if (velo.y < 0) {
            velo.y = fabs(y_velo) * elasticity;
            body_set_velocity(body, velo);
        }
    }
    list_free(shape);
}

void add_star(scene_t *scene, body_t *anchor, int pts){
    vector_t center = (vector_t) { MIN.x, MAX.y };
    body_t *new_star = make_star(STAR_SIZE, center, pts);
    body_set_velocity(new_star, INIT_VELO);
    scene_add_body(scene, new_star);
    create_newtonian_gravity(scene, G, new_star, anchor);
}

int main() {
    scene_t *scene = scene_init();
    sdl_init(MIN, MAX);
    srand(time(NULL));

    body_t *anchor = make_ground();
    scene_add_body(scene, anchor);

    double counter = 0;
    int pts_count = INIT_PTS;
    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        if (counter > TIME_BETWEEN_STARS) {
            add_star(scene, anchor, pts_count);
            pts_count += 2;
            counter = 0;
        }
        scene_tick(scene, dt);
        body_set_velocity(anchor, (vector_t) {0, 0});
        for (size_t i = 0; i < scene_bodies(scene); i++) {
            on_ground(scene_get_body(scene, i), ELASTICITY);
        }
        sdl_render_scene(scene);
        counter += dt;
    }
    scene_free(scene);
    return 0;
}
