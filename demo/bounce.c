#include "sdl_wrapper.h"
#include "body.h"
#include <math.h>
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include "color.h"
#include "scene.h"

const vector_t MIN = {0.0, 0.0};
const vector_t MAX = {200.0, 100.0};
const vector_t VELO = { 50, 20 };
const double ANGLE = 0.01;
const double MASS = 1.0;
const int NUM_SIDES = 10;
const int STAR_SIZE = 10;
const double PI = 3.14159265;
const rgb_color_t YELLOW = {(float)1.0, (float)1.0, (float)0.0};

body_t *make_star(double size, vector_t center, int num_sides) {
  list_t *list = list_init(num_sides, (free_func_t) free);
    double angle = 0.0;
    for (int i = 0; i < num_sides; i+= 2) {
        vector_t value1 = { cos(angle) * size, sin(angle) * size };
	    vector_t *v_1 = malloc(sizeof(vector_t));
	    *v_1 = vec_add(value1, center);
	    list_add(list, v_1);
        angle += (2 * PI / num_sides);

	    vector_t value2 = { cos(angle) * size / 2, sin(angle) * size / 2 };
	    vector_t *v_2 = malloc(sizeof(vector_t));
	    *v_2 = vec_add(value2, center);
	    list_add(list, v_2);
	    angle += (2 * PI / num_sides);
    }
    return body_init(list, MASS, YELLOW);
}

void wall_collision(body_t *body) {
    list_t *shape = body_get_shape(body);
    vector_t velo = body_get_velocity(body);
    double x_velo = velo.x;
    double y_velo = velo.y;
    for (size_t i = 0; i < list_size(shape); i++) {
        vector_t *current_vec = list_get(shape, i);
        if (current_vec->x > MAX.x) {
            velo.x = -fabs(x_velo);
        }
        else if (current_vec->x < VEC_ZERO.x) {
            velo.x = fabs(x_velo);
        }
        if (current_vec->y > MAX.y) {
            velo.y = -fabs(y_velo);
        }
        else if (current_vec->y < VEC_ZERO.y) {
            velo.y = fabs(y_velo);
        }
        body_set_velocity(body, velo);
    }
    list_free(shape);
}

void update_star(scene_t *scene, double dt, double angle) {
    body_t *star = scene_get_body(scene, 0);
    double new_angle = body_get_rotation(star);
    scene_tick(scene, dt);
    new_angle += angle;
    body_set_rotation(star, new_angle);
    wall_collision(star);
}

int main() {
    scene_t *scene = scene_init();
    vector_t center = vec_multiply(THIS_IS_A_HALF, MAX);
    sdl_init(MIN, MAX);
    body_t *star = make_star(STAR_SIZE, center, NUM_SIDES);
    body_set_velocity(star, VELO);
    scene_add_body(scene, star);
    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        update_star(scene, dt, ANGLE);
        sdl_render_scene(scene);
    }
    scene_free(scene);
    return 0;
}
