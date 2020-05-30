#ifndef __HEALTH_H__
#define __HEALTH_H__
// In this file we define how we render the health bars of the tanks

#include "tank.h"

// Creates a new health bar for the tank and assigns the health_bar field of t1
// to the new health bar
void health_init(tank_t *t1);


// Updates the health bar of a tank
void update_health_bar(tank_t *t1);




#endif // #ifndef __HEALTH_H__
