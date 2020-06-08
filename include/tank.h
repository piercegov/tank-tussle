#ifndef __TANK_H__
#define __TANK_H__

#include "list.h"
#include "vector.h"
#include "render_health.h"
#include "power_bar.h"
#include "body.h"
#include <stdbool.h>
#include "fuel_bar.h"

typedef struct tank tank_t;

typedef struct tank_info {
    health_bar_t *health_bar;
    power_bar_t *power_bar;
    fuel_bar_t *fuel_bar;
    body_t *barrel;
    double angle;
    double power;
    double health;
    double fuel;
    bool turn;
    int tank_number;
} tank_info_t;

body_t *tank_init(double mass, rgb_color_t color, vector_t center, double size, int tank_num, vector_t offset);

tank_info_t *tank_get_info(body_t *tank);

int tank_get_number(body_t *tank);

health_bar_t *tank_get_health_bar(body_t *tank);

double tank_get_health(body_t *tank);

void tank_decrease_health(body_t *tank, double health_decrease);

void tank_decrease_fuel(body_t *tank, double fuel_decrease);

fuel_bar_t *tank_get_fuel_bar(body_t *tank);

double tank_get_fuel(body_t *tank);

void tank_set_fuel(body_t *tank, double new_fuel);

double tank_get_power(body_t *tank);

power_bar_t *tank_get_power_bar(body_t *tank);

double tank_get_angle(body_t *tank);

void tank_set_power(body_t *tank, double new_power);

void tank_set_angle(body_t *tank, double new_angle);

bool tank_is_dead(body_t *tank);

bool tank_get_turn(body_t *tank);

void tank_set_turn(body_t *tank, bool turn);

body_t *tank_get_barrel(body_t *tank);

#endif // #ifndef __TANK_H__
