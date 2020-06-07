#ifndef __POWER_BAR_H__
#define __POWER_BAR_H__
// In this file we define how we render the health bars of the tanks

#include "body.h"

typedef struct power_bar {
    body_t *outer;
    body_t *inner;
    body_t *power_level;
} power_bar_t;

// Creates a new power bar for the tank and assigns the power_bar field of t1
// to the new power bar
power_bar_t *power_bar_init(body_t *t1, vector_t center);

// Updates the power bar of a tank
void update_power_bar(body_t *t1);

#endif // #ifndef __POWER_BAR_H__
