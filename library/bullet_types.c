#include "bullet_type.h"
#include "body.h"
#include "list.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "tank.h"
#include "polygon.h"
#include "math.h"

const double BASE_POWER = 10.0;
const vector_t BULLET_SPRITE_SIZE = {16.0, 9.0};
const double CLUSTER_OFF = 5.0;
const double PIIII = 3.14159265;
const vector_t CLUSTER_VELO = 2.0;

typedef struct cluster_aux {
    scene_t *scene;
    int num_clusters;
    double dmg;
    double wind;
} cluster_aux_t;

typedef struct kinetic_bullet_aux {
    double damage;
} kinetic_bullet_aux_t;

void calc_cluster_bomb_collision(body_t *body1, body_t *body2, vector_t axis, cluster_aux_t *aux) {
    int num_clusters = aux->num_clusters;
    scene_t *scene = aux->scene;
    double rads_per = 2 * PIIII / num_clusters;
    for (size_t i = 0; i < num_clusters; i++) {
        vector_t velo = vec_multiply(CLUSTER_VELO, (vector_t) {cos(i * rads_per), sin(i*rads_per)});
        vector_t pos = vec_multiply(CLUSTER_OFF, (vector_t) {cos(i * rads_per), sin(i*rads_per)});
        create_kinetic_bullet(scene, body1, body2, vec_add(pos, body_get_centroid(body2)), velo, aux->wind, aux->dmg);
    }
}

void create_cluster_bomb_collision(scene_t *scene, body_t *tank, body_t *bullet, int num_clusters, double dmg,
        double wind) {
    cluster_aux_t *aux = malloc(sizeof(cluster_aux_t));
    aux->scene = scene;
    aux->num_clusters = num_clusters;
    aux->dmg = dmg;
    aux->wind = wind;
    create_terrain_collision(scene, scene->terrain, bullet, (collision_handler_t) calc_cluster_bomb_collision, aux,
        (free_func_t) free);
    create_collision(scene, tank, bullet, (collision_handler_t) calc_cluster_bomb_collision, aux,
        (free_func_t) free);
}

void change_turn(body_t *cur_tank, body_t *next_tank) {
    double current_fuel = tank_get_fuel(next_tank);
    if (current_fuel + NEW_FUEL > 100) {
        tank_set_fuel(next_tank, 100.0);
    }
    else {
        tank_set_fuel(next_tank, current_fuel + NEW_FUEL);
    }
    tank_set_turn(next_tank, true);
    update_fuel_bar(next_tank);
    tank_set_turn(cur_tank, false);
}

body_t *init_gen_bullet(scene_t *scene, body_t *t1, body_t *t2, vector_t pos, vector_t velo,
        double wind, double dmg) {
    list_t *points = create_arc(BULLET_SIZE, 2*PI);
    bullet_info_t *bullet_aux = malloc(sizeof(kinetic_bullet_aux_t));
    bullet_aux->damage = dmg;
    body_t *bullet = body_init_with_info(points, 1.0, BLACK, (void *) kinetic_bullet_aux_t, free);
    body_set_type(bullet, 1);
    // vector_t tank_center = body_get_centroid(t1);
    // body_set_centroid(bullet, tank_center);
    body_set_centroid(bullet, pos);

    // double angle = tank_get_angle(t1);
    // angle = (angle * PI) / 180.0;
    // double x_dir = cos(angle);
    // double y_dir = sin(angle);
    //
    // double power = tank_get_power(tank);
    // power = power + BASE_POWER;
    // vector_t velo = vec_multiply(power, (vector_t) {x_dir, y_dir});

    int type = tank_get_number(t1);
    if (type == 1) {
        body_set_velocity(bullet, velo);
    }
    else {
        //If the tank is on the righthand side then the x-velo needs to be flipped
        velo.x = velo.x * -1;
        body_set_velocity(bullet, velo);
    }
    SDL_Texture *bullet_text = sdl_create_sprite_texture("images/bullet.png");
    add_texture(bullet, bullet_text, BULLET_SPRITE_SIZE);
    scene_add_body(scene, bullet);
    create_oneway_destructive_collision(scene, left_wall, bullet);
    create_oneway_destructive_collision(scene, right_wall, bullet);
    create_bullet_rotate(scene, bullet);
    create_drag(scene, wind, bullet);

    body_t *other_tank;
    body_t *cur_tank;
    if (type == 1) {
        other_tank = t1;
        cur_tank = t2;
    }
    else {
        other_tank = t2;
        cur_tank = t1;
    }
    create_damaging_collision(scene, other_tank, bullet);
    change_turn(cur_tank, other_tank);

    return bullet;
}

void create_kinetic_bullet(scene_t *scene, body_t *t1, body_t *t2, vector_t pos, vector_t velo,
        double wind, double dmg) {
    body_t *bullet = init_gen_bullet(scene, t1, t2, pos, velo, wind, dmg);
    create_bullet_destroy(scene, terrain, bullet);
    create_newtonian_gravity(scene, G, bullet, anchor);
}

void create_cluster_bomb(scene_t *scene, body_t *t1, body_t *t2,
        body_t *anchor, body_t *left_wall, body_t *right_wall, double wind, double dmg) {
    body_t *bullet = init_gen_bullet(scene, t1, t2, left_wall, right_wall, wind, dmg);
    create_newtonian_gravity(scene, G, bullet, anchor);

}
