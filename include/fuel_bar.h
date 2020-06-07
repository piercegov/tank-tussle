#ifndef __FUEL_BAR_H__
#define __FUEL_BAR_H__
// In this file we define how we render the health bars of the tanks

#include "body.h"

typedef struct fuel_bar {
    body_t *outer;
    body_t *inner;
    body_t *fuel_level;
} fuel_bar_t;

// Creates a new fuel bar for the tank and assigns the fuel_bar field of t1
// to the new fuel bar
fuel_bar_t *fuel_bar_init(body_t *t1, vector_t center);

// Updates the fuel bar of a tank
void update_fuel_bar(body_t *t1);

#endif // #ifndef __FUEL_BAR_H__
