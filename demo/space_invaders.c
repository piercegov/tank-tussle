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

#define BULLET 0
#define SHOOTER 1
#define ALIEN 2


// Cartesian Coordinates (not pixel values)
const vector_t MIN = {0.0, 0.0};
const vector_t MAX = {200.0, 100.0};

const double CIRC_DENSITY = 10.0;
const double PI = 3.14159265;
const vector_t START_POS = {100.0, 5.0};
const vector_t SHOOTER_VELO = {200.0, 0};
const vector_t BULLET_VELO = {0, 250.0};
const vector_t ALIEN_VELO = {50.0, 0.0};
const double MASS = 1.0;

const rgb_color_t GRAY = {0.5, 0.5, 0.5};
const rgb_color_t GREEN = {0.137, 0.639, 0.11};
const rgb_color_t BLACK = {0.0, 0.0, 0.0};
const double INTERVAL = 1.0;
const double SHOOTER_SIZE = 5.0;
const double BULLET_SIZE = 5.0;
const int NUM_ROWS = 3;
const int NUM_COLS = 10;


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

body_t *create_ellipse(double d, double rads, int *is_shooter) {
    double total_pts = CIRC_DENSITY * rads;
    list_t *points = list_init(total_pts, (free_func_t) free);
    double angle = 0.0;
    for (size_t i = 0; i < total_pts; i++) {
        vector_t *new_pt = malloc(sizeof(vector_t));
        *new_pt = (vector_t) { SHOOTER_SIZE * cos(angle), (SHOOTER_SIZE / 2) * sin(angle) };
        list_add(points, new_pt);
        angle += (1 / CIRC_DENSITY);
    }
    return body_init_with_info(points, MASS, GREEN, is_shooter, free);
}

void add_shooter(scene_t *scene) {
    int *is_shooter = malloc(sizeof(int));
    *is_shooter = SHOOTER;
    body_t *shooter = create_ellipse(SHOOTER_SIZE, 2*PI, is_shooter);
    body_set_centroid(shooter, START_POS);
    scene_add_body(scene, shooter);
}

void add_aliens(scene_t *scene) {
    int cols = NUM_COLS;
    double center_x = .75 * SHOOTER_SIZE;
    double center_y = MAX.y - SHOOTER_SIZE;
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < NUM_ROWS; j++) {
            int *is_shooter = malloc(sizeof(int));
            *is_shooter = ALIEN;

            vector_t center = { center_x, center_y };
            list_t *points = create_arc(SHOOTER_SIZE, PI);
            body_t *alien = body_init_with_info(points, MASS, GRAY, is_shooter, free);
            body_set_centroid(alien, center);

            vector_t *center_pt = malloc(sizeof(vector_t));
            assert(center_pt != NULL);

            *center_pt = center;
            center_pt->y -= 0.5 * SHOOTER_SIZE;
            list_add(points, (void *) center_pt);

            body_set_velocity(alien, ALIEN_VELO);
            scene_add_body(scene, alien);
            center_y -= (1.5 * SHOOTER_SIZE);
        }
        center_x += (1.5 * SHOOTER_SIZE);
        center_y = MAX.y - SHOOTER_SIZE;
    }

}


list_t *make_bullet() {
    list_t *points = list_init(4, (free_func_t) free);
    vector_t *p1 = malloc(sizeof(vector_t));
    vector_t *p2 = malloc(sizeof(vector_t));
    vector_t *p3 = malloc(sizeof(vector_t));
    vector_t *p4 = malloc(sizeof(vector_t));
    assert(p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL);
    *p1 = (vector_t) {0.0, 0.0};
    *p2 = (vector_t) {0.5, 0.0};
    *p3 = (vector_t) {0.5, 4.0};
    *p4 = (vector_t) {0.0, 4.0};
    list_add(points, p1);
    list_add(points, p2);
    list_add(points, p3);
    list_add(points, p4);
    return points;
}

void render_shooter(vector_t velocity, scene_t *scene) {
    body_t *shooter = scene_get_body(scene, 0);
    body_set_velocity(shooter, velocity);
}

void shoot_bullet(scene_t *scene, body_t *shooter, vector_t velo, rgb_color_t color) {
    list_t *points = make_bullet();
    int *is_shooter = malloc(sizeof(int));
    *is_shooter = BULLET;
    body_t *bullet = body_init_with_info(points, MASS, color, is_shooter, free);
    body_set_centroid(bullet, body_get_centroid(shooter));
    body_set_velocity(bullet, velo);
    scene_add_body(scene, bullet);

    if (*((int *)body_get_info(shooter)) == SHOOTER) {
        for (int i = 1; i < scene_bodies(scene); i++) {
            body_t *current = scene_get_body(scene, i);
            if (*(int *)body_get_info(current) == ALIEN) {
                create_destructive_collision(scene, bullet, scene_get_body(scene, i));
            }
        }
    }
    else if (*((int *)body_get_info(shooter)) == ALIEN){
        create_destructive_collision(scene, scene_get_body(scene, 0), bullet);
    }
}

void shooter_key_handler(char key, key_event_type_t type, double held_time, scene_t *scene) {
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                render_shooter(vec_multiply(-1, SHOOTER_VELO), scene);
                break;

            case RIGHT_ARROW:
                render_shooter(SHOOTER_VELO, scene);
                break;

            case SPACE_BAR:
                shoot_bullet(scene, scene_get_body(scene, 0), BULLET_VELO, BLACK);
                break;
        }
    }
    else if (type == KEY_RELEASED) {
        render_shooter((vector_t){ 0 , 0 }, scene);
    }
}

int update_aliens(scene_t *scene) {
    int counter = 0;
    for (size_t i = 1; i < scene_bodies(scene); i++) {
        body_t *current = scene_get_body(scene, i);
        if (*(int *)body_get_info(current) == ALIEN) {
            counter++;
            vector_t center = body_get_centroid(current);
            if (body_get_velocity(current).x < 0) {
                if (center.x <= MIN.x) {
                    center.y -= NUM_ROWS * 1.5 * SHOOTER_SIZE;
                    body_set_centroid(current, center);
                    body_set_velocity(current, vec_multiply(-1, body_get_velocity(current)));
                }
            }
            if (body_get_velocity(current).x > 0) {
                if (center.x >= MAX.x) {
                    center.y -= NUM_ROWS * 1.5 * SHOOTER_SIZE;
                    body_set_centroid(current, center);
                    body_set_velocity(current, vec_multiply(-1, body_get_velocity(current)));
                }
            }
        }
    }
    return counter;
}

void terminate_off_screen(scene_t *scene) {
    for (int i = 1; i < scene_bodies(scene); i++) {
        body_t *shooter = scene_get_body(scene, i);
        if (*((int *)body_get_info(shooter)) == BULLET) {
            vector_t center = body_get_centroid(shooter);
            if (center.y < MIN.y || center.y > MAX.y) {
                body_remove(shooter);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    scene_t *scene = scene_init();
    srand(time(NULL));
    double time_since_last_add = 0.0;
    int num_aliens = 1;
    add_shooter(scene);
    add_aliens(scene);
    sdl_init(MIN, MAX);
    sdl_on_key((key_handler_t) shooter_key_handler);
    while (!sdl_is_done(scene)) {
        if ((*(int *) body_get_info(scene_get_body(scene, 0)) != SHOOTER) || num_aliens == 0) {
            break;
        }
        double dt = time_since_last_tick();
        terminate_off_screen(scene);
        num_aliens = update_aliens(scene);
        scene_tick(scene, dt);
        time_since_last_add += dt;
        if (time_since_last_add > INTERVAL) {
            while (num_aliens > 0) {
                int random = (rand() % (num_aliens) + 1);
                body_t *bod = scene_get_body(scene, random);
                if (*(int *)body_get_info(bod) == ALIEN) {
                    shoot_bullet(scene, bod,
                            vec_multiply(-1, BULLET_VELO), BLACK);
                    break;
                }
            }
            time_since_last_add = 0.0;
        }

        sdl_render_scene(scene);
    }
    scene_free(scene);
}
