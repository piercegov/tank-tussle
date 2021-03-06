#include "body.h"
#include "vector.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "polygon.h"
#include "interval.h"
#include <stdbool.h>
#include "collision.h"
#include <SDL2/SDL_image.h>

typedef struct body {
    list_t *points;
    vector_t centroid;
    rgb_color_t color;
    double mass;
    vector_t *velocity;
    vector_t acceleration;
    double rotation;
    double elasticity;
    vector_t force;
    vector_t impulse;
    void *info;
    int type;
    free_func_t freer;
    bool remove;
    SDL_Texture *texture;
    vector_t texture_size;
    SDL_Rect *texture_rect;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
    return body_init_with_info(shape, mass, color, NULL, NULL);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color, void *info, free_func_t info_freer) {
    body_t *body = malloc(sizeof(body_t));
    vector_t *velo = malloc(sizeof(vector_t));
    SDL_Rect *rect = malloc(sizeof(SDL_Rect));
    assert(body != NULL);
    assert(velo != NULL);
    assert(mass > 0);

    body->info = info;
    body->type = 0;
    body->texture = NULL;
    body->freer = info_freer;
    body->points = shape;
    body->color = color;
    body->mass = mass;
    body->centroid = polygon_centroid(shape);
    *velo = VEC_ZERO;
    body->velocity =  velo;
    body->acceleration = VEC_ZERO;
    body->rotation = 0.0;
    body->elasticity = 1.0;
    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;
    body->texture = NULL;
    body->texture_size = VEC_ZERO;
    body->texture_rect = rect;
    return body;
}

void body_free(body_t *body) {
    list_free(body->points);
    free(body->velocity);
    free(body->texture_rect);
    free_func_t info_free = body->freer;
    if (body->info != NULL && info_free != NULL){
        info_free(body->info);
    }
    if (body->texture != NULL) {
        SDL_DestroyTexture(body->texture);
    }
    free(body);
}

list_t *body_get_points(body_t *body) {
    return body->points;
}

void *body_get_info(body_t *body) {
    return body->info;
}

double body_get_mass(body_t *body){
    return body->mass;
}

double body_get_rotation(body_t *body) {
    return body->rotation;
}

list_t *body_get_shape(body_t *body) {
    size_t num_points = list_size(body->points);
    list_t *shape = list_init(num_points, (free_func_t) free);

    for (size_t i = 0; i < num_points; i++) {
        // Not correct, need to create new vector pointers
        vector_t *point = malloc(sizeof(vector_t));
        assert(point != NULL);
        *point = *(vector_t *)(list_get(body->points, i));
        list_add(shape, point);
    }

    return shape;
}

vector_t body_get_centroid(body_t *body) {
    return body->centroid;
}

void body_set_color(body_t *body, rgb_color_t color) {
    body->color = color;
}

vector_t body_get_velocity(body_t *body) {
    return *(body->velocity);
}

rgb_color_t body_get_color(body_t *body) {
    return body->color;
}

void body_set_rotation_relative(body_t *body, double angle, vector_t point) {
    list_t *pts = body_get_points(body);
    double angle_difference = angle - body->rotation;
    polygon_rotate(pts, angle_difference, point);
    body->rotation = angle;
    body_set_centroid(body, polygon_centroid(pts));
}

void body_set_centroid(body_t *body, vector_t x) {
    list_t *points = body_get_points(body);
    vector_t old_center = body_get_centroid(body);
    vector_t difference = vec_subtract(x, old_center);
    polygon_translate(points, difference);
    body->centroid = x;
}

vector_t body_vec_between(body_t *body1, body_t *body2){
    vector_t r12 = vec_subtract(body_get_centroid(body1), body_get_centroid(body2));
    return r12;
}

void body_set_velocity(body_t *body, vector_t v) {
    *(body->velocity) = v;
}

void body_set_rotation(body_t *body, double angle) {
    double angle_difference = angle - body->rotation;
    polygon_rotate(body_get_points(body), angle_difference, body_get_centroid(body));
    body->rotation = angle;
}

void body_add_force(body_t *body, vector_t force) {
    body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
    body->impulse = vec_add(body->impulse, impulse);
}

vector_t body_check_collision(body_t *b1, body_t *b2) {
    return (find_collision(b1->points, b2->points)).axis;
}

void body_tick(body_t *body, double dt) {
    body->acceleration = vec_multiply((1 / body->mass), body->force);
    vector_t dv = vec_add(vec_multiply(dt, body->acceleration), vec_multiply(1/body->mass, body->impulse));

    vector_t velocity = *body->velocity;
    vector_t displacement = vec_multiply(dt, vec_average(vec_add(velocity, dv), velocity));
    vector_t new_centroid = vec_add(displacement, body_get_centroid(body));

    body_set_centroid(body, new_centroid);
    body_set_velocity(body, vec_add(velocity, dv));

    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;
}

void body_remove(body_t *body) {
    if (!(body->remove)) {
        body->remove = true;
    }
}

bool body_is_removed(body_t *body) {
    return body->remove;
}

bool body_is_colliding(body_t *body1, body_t *body2) {
    // DEPRECATED
    return vec_isequal(body_check_collision(body1, body2), VEC_ZERO);
}

vector_t body_get_max(body_t *b, double min_x, double max_x) {
    list_t *points = body_get_points(b);
    vector_t max = body_get_centroid(b);
    for (size_t i = 0; i < list_size(points); i++) {
        vector_t *current = list_get(points, i);
        if (current->x >= min_x && current->x <= max_x && current->y > max.y) {
            max = *current;
        }
    }
    return max;
}

vector_t body_get_min(body_t *b, double min_x, double max_x) {
    list_t *points = body_get_points(b);
    vector_t min = body_get_centroid(b);
    for (size_t i = 0; i < list_size(points); i++) {
        vector_t *current = list_get(points, i);
        if (current->x >= min_x && current->x <= max_x && current->y < min.y) {
            min = *current;
        }
    }
    return min;
}

SDL_Texture *body_get_texture(body_t *b) {
    return b->texture;
}

vector_t body_get_texture_size(body_t *b) {
    return b->texture_size;
}

void body_set_texture(body_t *b, SDL_Texture *texture, vector_t sprite_size) {
    b->texture = texture;
    b->texture_size = sprite_size;
}

SDL_Rect *body_get_texture_rect(body_t *b) {
    return b->texture_rect;
}

void body_set_texture_rect(body_t *b, vector_t upper_left, vector_t dimensions) {
    b->texture_rect->x = upper_left.x;
    b->texture_rect->y = upper_left.y;
    b->texture_rect->w = dimensions.x;
    b->texture_rect->h = dimensions.y;
}

int body_get_type(body_t *b) {
    return b->type;
}

void body_set_type(body_t *b, int type) {
    b->type = type;
}
