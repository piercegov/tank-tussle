#include "list.h"
#include <assert.h>
#include <stdlib.h>
#include "body.h"
#include "forces.h"
#include <stdio.h>
#include <stdbool.h>

const int INIT_BODIES = 5;

typedef struct scene {
    list_t *bodies;
    list_t *forces;
    body_t *tank1;
    body_t *tank2;
    body_t *terrain;
    body_t *left_wall;
    body_t *right_wall;
    body_t *anchor;
} scene_t;

void scene_anchor(scene_t *scene, body_t *anchor) {
    scene->anchor = anchor;
}

void scene_walls(scene_t *scene, body_t *left_wall, body_t *right_wall) {
    scene->left_wall = left_wall;
    scene->right_wall = right_wall;
}

void scene_terrain(scene_t *scene, body_t *terrain) {
    scene->terrain= terrain;
}

void scene_tanks(scene_t *scene, body_t *t1, body_t *t2) {
    scene->tank1 = t1;
    scene->tank2 = t2;
}

body_t *scene_get_anchor(scene_t *scene) {
    return scene->anchor;
}

body_t *scene_get_left(scene_t *scene) {
    return scene->left_wall;
}

body_t *scene_get_right(scene_t *scene) {
    return scene->right_wall;
}

body_t *scene_get_terrain(scene_t *scene) {
    return scene->terrain;
}

body_t *scene_get_tank1(scene_t *scene) {
    return scene->tank1;
}

body_t *scene_get_tank2(scene_t *scene) {
    return scene->tank2;
}

scene_t *scene_init() {
    scene_t *scene = malloc(sizeof(scene_t));
    list_t *bodies = list_init(INIT_BODIES, (free_func_t) body_free);
    list_t *forces = list_init(INIT_BODIES, (free_func_t) force_free);

    assert(scene != NULL);

    scene->bodies = bodies;
    scene->forces = forces;

    return scene;
}

void scene_free(scene_t *scene) {
    list_free(scene->bodies);
    list_free(scene->forces);
    free(scene);
}

size_t scene_bodies(scene_t *scene){
    return list_size(scene->bodies);
}

body_t *scene_get_body(scene_t *scene, size_t index){
    return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body){
    list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index){
    body_remove(scene_get_body(scene, index));
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
    scene_add_bodies_force_creator(scene, forcer, aux, NULL, freer);
}

void scene_add_bodies_force_creator(
    scene_t *scene,
    force_creator_t forcer,
    void *aux,
    list_t *bodies,
    free_func_t freer
) {
    force_t *new_force = force_init(forcer, aux, bodies, freer);
    list_add(scene->forces, new_force);
}

void del_force(scene_t *scene, size_t index, force_t *force) {
    list_t *force_bodies = force_get_bodies(force);
    if (force_bodies == NULL) {
        return;
    }
    for (size_t j = 0; j < list_size(force_bodies); j++) {
        body_t *current_body = list_get(force_bodies, j);
        if (body_is_removed(current_body)) {
            force_t *removed_force = list_remove(scene->forces, index);
            force_free(removed_force);
            return;
        }
    }
}

void delete_forces(scene_t *scene) {
    for (int i = list_size(scene->forces) - 1; i >= 0; i--) {
        force_t *force = list_get(scene->forces, i);
        del_force(scene, i, force);
    }
}

void update_forces(scene_t *scene) {
    list_t *scene_forces = scene->forces;
    for (size_t i = 0; i < list_size(scene_forces); i++) {
        force_t *force = list_get(scene_forces, i);

        void *aux = force_get_aux(force);
        force_creator_t creator = force_get_forcer(force);
        creator(aux);
    }
}

void update_bodies(scene_t *scene, double dt) {
    for (size_t k = 0; k < list_size(scene->bodies); k++) {
        body_t *current_body = scene_get_body(scene, k);
        if (body_is_removed(current_body)) {
            body_t *removed = list_remove(scene->bodies, k);
            body_free(removed);
            k--;
        }
        else {
            body_tick(current_body, dt);
        }
    }
}

void scene_tick(scene_t *scene, double dt) {
    update_forces(scene);
    delete_forces(scene);
    update_bodies(scene, dt);
}
