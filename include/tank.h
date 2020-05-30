#ifndef __TANK_H__
#define __TANK_H__

#include "list.h"
#include "vector.h"
#include "body.h"

typedef struct tank tank_t;

tank_t *tank_init(double max_health, double mass, rgb_color_t color);

void tank_free(tank_t *tank);

body_t *tank_get_body(tank_t *tank);

health_t *tank_get_health_bar(tank_t *tank);

void *tank_get_info(tank_t *tank);

void tank_set_info(tank_t *tank, void* info);

double tank_get_health(tank_t *tank);

void tank_set_health(tank_t *tank, double health_decrease);

double tank_get_power(tank_t *tank);

double tank_get_angle(tank_t *tank);

void tank_set_power(tank_t *tank, double new_power);

void tank_set_angle(tank_t *tank, double new_angle);

bool tank_is_destroyed(tank_t *tank);




#endif // #ifndef __TANK_H__
