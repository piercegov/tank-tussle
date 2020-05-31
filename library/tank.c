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

const double PI = 3.14159265;

typedef struct tank_info {
    health_bar_t *health_bar;
    double angle;
    double power;
    double health;
    int tank_number;
} tank_info_t;

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

// When we add the tank to the scene we also need to make sure that we add its health bar
body_t *tank_init(double mass, rgb_color_t color, vector_t center, double size, int tank_num) {
    list_t *points = create_arc(size, 2*PI);
    tank_info_t *info = malloc(sizeof(tank_info_t));
    info->health_bar = *health_init(tank);
    info->angle = 0.0;
    info->power = 0.0;
    info->health = 1.0;
    info->tank_number = tank_num;

    // Health Bar freed in scene free
    body_t *tank = body_init_with_info(points, mass, color, info, free);
    body_set_centroid(tank, center);
    return

}

tank_info_t *tank_get_info(body_t *tank) {
    return (tank_info_t)body_get_info(tank);
}

int tank_get_number(body_t *tank) {
    return tank_get_info(tank)->tank_number;
}

health_t *tank_get_health_bar(body_t *tank) {
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

double tank_get_angle(body_t *tank) {
    return tank_get_info(tank)->health;
}

void tank_set_power(body_t *tank, double new_power) {
    tank_info_t *info = tank_get_info(tank);
    info->power = new_power;
}

void tank_set_angle(body_t *tank, double new_angle) {
    tank_info_t *info = tank_get_info(tank);
    info->angle = new_angle;
}

bool tank_is_destroyed(body_t *tank) {
    if (tank_get_info(tank)->health <= 0.0) {
        return true;
    }
    return false;
}
