#ifndef __BULLET_TYPE_H__
#define __BULLET_TYPE_H__

#include "body.h"
#include "forces.h"
#include "scene.h"
const double FUEL_CONSTANT = 10.0;


void create_kinetic_bullet(scene_t *scene, body_t *t1, body_t *t2, vector_t pos, vector_t velo,
        double wind, double dmg);


void create_cluster_bomb(scene_t *scene, body_t *t1, body_t *t2, vector_t pos, vector_t velo,
        double wind, double dmg);

#endif // #ifndef __TERRAIN_H__
