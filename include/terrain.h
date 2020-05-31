#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "body.h"

body_t *generate_terrain(double width, double base_height, double scale, int granularity, double damping, double mass);

#endif // #ifndef __TERRAIN_H__
