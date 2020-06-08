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
#include "tank.h"
#include "render_health.h"
#include "power_bar.h"
#include "fuel_bar.h"
#include "color.h"

const double TANK_HEIGHT = 2.5;
const double TANK_WIDTH = 4.5;
const double BARREL_LENGTH = 3.0;
const double BARREL_HEIGHT = 1.0;
const vector_t POWER_LEFT = {20.0, 15.0};
const vector_t POWER_RIGHT = {180.0, 15.0};
const vector_t FUEL_LEFT = {20.0, 5.0};
const vector_t FUEL_RIGHT = {180.0, 5.0};
const rgb_color_t BLACK = {0.0, 0.0, 0.0};
const rgb_color_t RED = {1.0, 0.0, 0.0};
const rgb_color_t GREEN = {0.0, 1.0, 0.0};
const rgb_color_t BLUE = {0.0, 0.0, 1.0};
const rgb_color_t WHITE = {1.0, 1.0, 1.0};
const double BAR_WIDTH = 15.0;
const double BAR_HEIGHT = 2.0;
const double OUTLINE_WIDTH = 0.5;

list_t *tank_hitbox(vector_t c, double w, double h) {
    list_t *points = create_rectangle(c, w, h);
    list_add(points, (void *)vec_init(c.x + (w/4.0), c.y + (h/3.0)));
    list_add(points, (void *)vec_init(c.x + (w/4.0), c.y + (1.25 * h)));
    list_add(points, (void *)vec_init(c.x - (w/4.0), c.y + (1.25 * h)));
    list_add(points, (void *)vec_init(c.x - (w/4.0), c.y + (h/3.0)));
    return points;
}

body_t *tank_init(double mass, rgb_color_t color, vector_t center, double size, int tank_num, vector_t barrel_dim) {
    list_t *points = tank_hitbox(center, TANK_WIDTH, TANK_HEIGHT);
    tank_info_t *info = malloc(sizeof(tank_info_t));

    info->angle = 0.0;
    info->power = 0.0;
    info->health = 100.0;
    info->fuel = 100.0;
    info->tank_number = tank_num;

    // Health Bar freed in scene free

    body_t *tank = body_init_with_info(points, mass, color, info, free);

    list_t *barrel_points = create_rectangle(vec_add(center, (vector_t){barrel_dim.x / 2, 0}), barrel_dim.x, barrel_dim.y);
    body_t *barrel = body_init(barrel_points, mass, color);

    info->barrel = barrel;
    info->health_bar = health_init(tank);

    // POG CHAMP
    vector_t power_pos;
    power_pos = (tank_num == 1) ? POWER_LEFT : POWER_RIGHT;
    info->power_bar = power_bar_init(tank, power_pos);

    vector_t fuel_pos;
    fuel_pos = (tank_num == 1) ? FUEL_LEFT : FUEL_RIGHT;
    info->fuel_bar = fuel_bar_init(tank, fuel_pos);

    return tank;

}

tank_info_t *tank_get_info(body_t *tank) {
    return (tank_info_t *)body_get_info(tank);
}

int tank_get_number(body_t *tank) {
    return tank_get_info(tank)->tank_number;
}

health_bar_t *tank_get_health_bar(body_t *tank) {
    return tank_get_info(tank)->health_bar;
}

double tank_get_health(body_t *tank) {
    return tank_get_info(tank)->health;
}

void tank_decrease_health(body_t *tank, double health_decrease) {
    tank_info_t *info = tank_get_info(tank);
    info->health -= health_decrease;
    update_health_bar(tank);
}

double tank_get_power(body_t *tank) {
    return tank_get_info(tank)->power;
}

power_bar_t *tank_get_power_bar(body_t *tank) {
    return tank_get_info(tank)->power_bar;
}

fuel_bar_t *tank_get_fuel_bar(body_t *tank) {
    return tank_get_info(tank)->fuel_bar;
}

double tank_get_fuel(body_t *tank) {
    return tank_get_info(tank)->fuel;
}

void tank_decrease_fuel(body_t *tank, double fuel_decrease) {
    tank_info_t *info = tank_get_info(tank);
    info->fuel -= fuel_decrease;
    update_fuel_bar(tank);
}

double tank_get_angle(body_t *tank) {
    return tank_get_info(tank)->angle;
}

void tank_set_power(body_t *tank, double new_power) {
    tank_info_t *info = tank_get_info(tank);
    info->power = new_power;
    update_power_bar(tank);
}

void tank_set_fuel(body_t *tank, double new_fuel) {
    tank_info_t *info = tank_get_info(tank);
    info->fuel = new_fuel;
    update_fuel_bar(tank);
}

void tank_set_angle(body_t *tank, double new_angle) {
    tank_info_t *info = tank_get_info(tank);
    info->angle = new_angle;
}

bool tank_is_dead(body_t *tank) {
    return (tank_get_info(tank)->health <= 0.0);
}

bool tank_get_turn(body_t *tank) {
    return tank_get_info(tank)->turn;
}

void tank_set_turn(body_t *tank, bool turn) {
    tank_get_info(tank)->turn = turn;
}

body_t *tank_get_barrel(body_t *tank) {
    return tank_get_info(tank)->barrel;
}
