#include "bullet_types.h"
#include "body.h"
#include "list.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "tank.h"
#include "polygon.h"
#include "math.h"

const vector_t BULLET_SPRITE_SIZE = {16.0, 9.0};
const double BULLET_MASS = 1.0;
const vector_t BOMB_SPRITE_SIZE = {30.0, 16.0};
const double CLUSTER_OFF = 5.0;
const double NUM_CLUSTERS = 6.0;
const double CLUSTER_DAMAGE = 10.0;
const double CLUSTER_VELO = 30.0;
const double G = 9001.0;
const double BULLET_SIZE = 1.0;
const double FUEL_CONSTANT = 10.0;

typedef struct cluster_aux {
    scene_t *scene;
    int num_clusters;
    double dmg;
    double wind;
    body_t *shooter;
    body_t *target;
} cluster_aux_t;

typedef struct kinetic_bullet_aux {
    double damage;
} kinetic_bullet_aux_t;

void add_bullet_texture(body_t *b, SDL_Texture *texture, vector_t sprite_size) {
    body_set_texture(b, texture, sprite_size);
    body_set_texture_rect(b, body_get_centroid(b), sprite_size);
}

void calc_cluster_bomb_collision(body_t *body1, body_t *bullet, vector_t axis, cluster_aux_t *aux) {
    int num_clusters = aux->num_clusters;
    scene_t *scene = aux->scene;
    double rads_per = 2 * PI / num_clusters;
    for (size_t i = 0; i < num_clusters; i++) {
        vector_t velo = vec_multiply(CLUSTER_VELO, (vector_t) {cos(i * rads_per), sin(i*rads_per)});
        vector_t pos = vec_multiply(CLUSTER_OFF, (vector_t) {cos(i * rads_per), sin(i*rads_per)});
        char path[] = "images/cherry_bomb.png";
        create_kinetic_bullet(scene, aux->target, aux->shooter, vec_add(pos, body_get_centroid(bullet)),
            velo, aux->wind, aux->dmg, path, BULLET_SPRITE_SIZE);
    }
    body_remove(bullet);
}

void create_cluster_bomb_collision(scene_t *scene, body_t *t1, body_t *t2, body_t *bullet, int num_clusters, double dmg,
        double wind) {
    cluster_aux_t *aux = malloc(sizeof(cluster_aux_t));
    aux->scene = scene;
    aux->num_clusters = num_clusters;
    aux->dmg = dmg;
    aux->wind = wind;
    aux->shooter = t2;
    aux->target = t1;
    create_terrain_collision(scene, scene_get_terrain(scene), bullet, (collision_handler_t) calc_cluster_bomb_collision, aux,
        (free_func_t) free);
    create_collision(scene, t1, bullet, (collision_handler_t) calc_cluster_bomb_collision, aux,
        (free_func_t) free);
}

void change_turn(scene_t *scene, body_t *cur_tank, body_t *next_tank) {
    double current_fuel = tank_get_fuel(next_tank);
    if (current_fuel + 10.0 > 100) {
        tank_set_fuel(next_tank, 100.0);
    }
    else {
        tank_set_fuel(next_tank, current_fuel + 10.0);
    }
    tank_set_turn(next_tank, true);
    update_fuel_bar(next_tank);
    tank_set_turn(cur_tank, false);
}

body_t *init_gen_bullet(scene_t *scene, body_t *t1, body_t *t2, vector_t pos, vector_t velo,
        double wind, double dmg) {
    list_t *points = create_arc(BULLET_SIZE, 2*PI);
    kinetic_bullet_aux_t *bullet_aux = malloc(sizeof(kinetic_bullet_aux_t));
    bullet_aux->damage = dmg;
    body_t *bullet = body_init_with_info(points, BULLET_MASS, BLACK, (void *) bullet_aux, free);
    body_set_type(bullet, 1);
    body_set_centroid(bullet, pos);
    body_set_velocity(bullet, velo);

    scene_add_body(scene, bullet);
    create_oneway_destructive_collision(scene, scene_get_left(scene), bullet);
    create_oneway_destructive_collision(scene, scene_get_right(scene), bullet);
    create_bullet_rotate(scene, bullet);
    create_drag(scene, wind, bullet);

    create_damaging_collision(scene, t1, bullet);
    change_turn(scene, t2, t1);

    return bullet;
}

void create_kinetic_bullet(scene_t *scene, body_t *t1, body_t *t2, vector_t pos, vector_t velo,
        double wind, double dmg, char path[], vector_t sprite_size) {
    body_t *bullet = init_gen_bullet(scene, t1, t2, pos, velo, wind, dmg);
    SDL_Texture *bullet_text = sdl_create_sprite_texture(path);
    add_bullet_texture(bullet, bullet_text, sprite_size);
    create_bullet_destroy(scene, scene_get_terrain(scene), bullet);
    create_newtonian_gravity(scene, G, bullet, scene_get_anchor(scene));
}

void create_cluster_bomb(scene_t *scene, body_t *t1, body_t *t2, vector_t pos, vector_t velo,
        double wind, double dmg) {
    body_t *bullet = init_gen_bullet(scene, t1, t2, pos, velo, wind, dmg);
    SDL_Texture *bullet_text = sdl_create_sprite_texture("images/cherry_bomb.png");
    add_bullet_texture(bullet, bullet_text, BOMB_SPRITE_SIZE);
    create_cluster_bomb_collision(scene, t1, t2, bullet, NUM_CLUSTERS, CLUSTER_DAMAGE, 0.0);
    create_newtonian_gravity(scene, G, bullet, scene_get_anchor(scene));
}

void create_multishot(scene_t *scene, body_t *t1, body_t *t2, vector_t pos, vector_t velo,
        double wind, double dmg, double num_bullets) {
    char path[] = "images/flash_bomb.png";
    for (int i = 0; i < num_bullets; i++) {
        create_kinetic_bullet(scene, t1, t2, pos, velo, wind, dmg, path, BULLET_SPRITE_SIZE);
        velo = vec_multiply(0.9, velo); //Reduce velocity to create spread
    }
}
