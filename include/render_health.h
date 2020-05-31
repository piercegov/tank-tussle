#ifndef __HEALTH_H__
#define __HEALTH_H__
// In this file we define how we render the health bars of the tanks

#include "body.h"

typedef struct health_bar {
    body_t *outer;
    body_t *inner;
    body_t *health_pool;
} health_bar_t;

// Creates a new health bar for the tank and assigns the health_bar field of t1
// to the new health bar
health_bar_t *health_init(body_t *t1);


// Updates the health bar of a tank
void update_health_bar(body_t *t1);




#endif // #ifndef __HEALTH_H__
