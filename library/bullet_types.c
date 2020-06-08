#include "bullet_type.h"
#include "body.h"
#include "list.h"
#include "scene.h"

typedef struct kinetic_bullet_aux {
    double damage;
} kinetic_bullet_aux_t;

void create_kinetic_bullet(scene_t *scene, body_t *t1, body_t *t2, double dmg) {
    list_t *points = create_arc(BULLET_SIZE, 2*PI);
    bullet_info_t *bullet_aux = malloc(sizeof(kinetic_bullet_aux_t));
    bullet_aux->damage = dmg;
    body_t *bullet = body_init_with_info(points, 1.0, BLACK, (void *) kinetic_bullet_aux_t, free);
    body_set_type(bullet, 1);
    vector_t tank_center = body_get_centroid(t1);
    body_set_centroid(bullet, tank_center);

    double angle = tank_get_angle(tank);
    angle = (angle * PI) / 180.0;
    double x_dir = cos(angle);
    double y_dir = sin(angle);

    double power = tank_get_power(tank);
    power = power + BASE_POWER;
    vector_t velo = vec_multiply(power, (vector_t) {x_dir, y_dir});

    int type = tank_get_number(tank);
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
    create_newtonian_gravity(scene, G, bullet, anchor);
    create_oneway_destructive_collision(scene, left_wall, bullet);
    create_oneway_destructive_collision(scene, right_wall, bullet);

    create_bullet_rotate(scene, bullet);
    create_drag(scene, wind, bullet);

    body_t *other_tank;
    if (type == 1) {
        other_tank = tank2;
    }
    else {
        other_tank = tank1;
    }
    create_damaging_collision(scene, other_tank, bullet);
    double current_fuel = tank_get_fuel(other_tank);
    if (current_fuel + NEW_FUEL > 100) {
        tank_set_fuel(other_tank, 100.0);
    }
    else {
        tank_set_fuel(other_tank, current_fuel + NEW_FUEL);
    }
    tank_set_turn(other_tank, true);
    update_fuel_bar(other_tank);
    create_bullet_destroy(scene, terrain, bullet);
    tank_set_turn(tank, false);
}
