#include "forces.h"
#include "collision.h"
#include "tank.h"
#include <stdlib.h>
#include "render_health.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "polygon.h"

const double MIN_DIST = 1.5;

typedef struct grav_aux {
    double G;
    body_t *body1;
    body_t *body2;
} grav_aux_t ;

typedef struct spring_aux {
    double K;
    body_t *body1;
    body_t *body2;
} spring_aux_t;

typedef struct drag_aux {
    double gamma;
    body_t *body;
} drag_aux_t;

typedef struct collision_aux {
    body_t *body1;
    body_t *body2;
    void *aux;
    bool has_collided;
    collision_handler_t handler;
    free_func_t freer;
} collision_aux_t;

typedef struct collision_terrain_aux {
    body_t *terrain;
    body_t *body;
    void *aux;
    bool has_collided;
    collision_handler_t handler;
    free_func_t freer;
} collision_terrain_aux_t;

typedef struct physics_aux {
    double elasticity;
} physics_aux_t;

typedef struct terrain_aux {
    body_t *terrain;
    body_t *body;
} terrain_aux_t;

typedef struct health_aux {
    body_t *tank;
} health_aux_t;

typedef struct bullet_aux {
    body_t *terrain;
    body_t *body;
} bullet_aux_t;

typedef struct rotate_aux {
    body_t *body;
} rotate_aux_t;

typedef struct barrel_rotate_aux {
    body_t *tank;
    vector_t offset;
} barrel_rotate_aux_t;

force_t *force_init(force_creator_t forcer, void *aux, list_t *bodies, free_func_t freer) {
    force_t *force = malloc(sizeof(force_t));
    assert(force != NULL);
    force->forcer = forcer;
    force->aux=aux;
    force->bodies = bodies;
    force->freer=freer;
    return force;
}

void *force_get_aux(force_t *force){
    return force->aux;
}

force_creator_t force_get_forcer(force_t *force){
    return force->forcer;
}

list_t *force_get_bodies(force_t *force) {
    return force->bodies;
}

free_func_t force_get_freer(force_t *force){
    return force->freer;
}

void grav_aux_free(grav_aux_t *aux){
    free(aux);
}

void spring_aux_free(spring_aux_t *aux){
    free(aux);
}

void drag_aux_free(drag_aux_t *aux){
    free(aux);
}

void collision_aux_free(collision_aux_t *aux){
    if (aux->freer != NULL) {
        (aux->freer)(aux->aux);
    }
    free(aux);
}

void collision_terrain_aux_free(collision_terrain_aux_t *aux){
    if (aux->freer != NULL) {
        (aux->freer)(aux->aux);
    }
    free(aux);
}

void physics_aux_free(physics_aux_t *aux){
    free(aux);
}

void force_free(force_t *force){
    if (force->freer != NULL) {
        (force->freer)(force->aux);
    }
    if (force->bodies != NULL) {
        list_free(force->bodies);
    }
    free(force);
}

void calc_newtonian_gravity(grav_aux_t *aux) {
    double G = aux->G;
    body_t *body1 = aux->body1;
    body_t *body2 = aux->body2;

    double m1 = body_get_mass(body1);
    double m2 = body_get_mass(body2);
    vector_t r12 = body_vec_between(body1, body2);
    double norm = vec_norm(r12);
    r12 = vec_multiply(1/norm, r12);

    if (norm < MIN_DIST) {
        return;
    }
    vector_t net_force21 = vec_multiply(G*m1*m2/(norm*norm), r12);
    vector_t net_force12 = vec_multiply(-1.0, net_force21);

    body_add_force(body1, net_force12);
    body_add_force(body2, net_force21);
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1, body_t *body2) {
    grav_aux_t *aux = malloc(sizeof(grav_aux_t));
    list_t *bodies = list_init(2, NULL);
    list_add(bodies, body1);
    list_add(bodies, body2);
    assert(aux != NULL);
    aux->G = G;
    aux->body1=body1;
    aux->body2=body2;
    scene_add_bodies_force_creator(
        scene,
        (force_creator_t) (calc_newtonian_gravity),
        aux,
        bodies,
        (free_func_t) (grav_aux_free)
    );

}

void calc_spring(spring_aux_t *aux) {
    double K = aux->K;
    body_t *body1 = aux->body1;
    body_t *body2 = aux->body2;

    vector_t r12 = body_vec_between(body1, body2);
    double norm = vec_norm(r12);
    if (norm < 1e-4) {
        return;
    }
    r12 = vec_multiply(norm, vec_multiply(1/vec_norm(r12), r12));
    vector_t net_force12 = vec_multiply(-1.0*K, r12);
    body_add_force(body1, net_force12);
    body_add_force(body2, vec_multiply(-1, net_force12));
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
    spring_aux_t *aux = malloc(sizeof(spring_aux_t));
    list_t *bodies = list_init(2, NULL);
    list_add(bodies, body1);
    list_add(bodies, body2);
    assert(aux != NULL);

    aux->K = k;
    aux->body1=body1;
    aux->body2=body2;
    scene_add_bodies_force_creator(
        scene,
        (force_creator_t) (calc_spring),
        aux,
        bodies,
        (free_func_t) (spring_aux_free)
    );
}

void calc_drag(drag_aux_t *aux) {
    double gamma = aux->gamma;
    body_t *body = aux->body;

    vector_t net_force = vec_multiply(-1.0*gamma, body_get_velocity(body));
    body_add_force(body, net_force);
}

void create_drag(scene_t *scene, double gamma, body_t *body){
    drag_aux_t *aux = malloc(sizeof(drag_aux_t));
    list_t *bodies = list_init(1, NULL);
    list_add(bodies, body);
    assert(aux != NULL);

    aux->gamma = gamma;
    aux->body=body;
    scene_add_bodies_force_creator(
        scene,
        (force_creator_t) (calc_drag),
        aux,
        bodies,
        (free_func_t) (drag_aux_free)
    );
}

// Collisions
// ----------------------------------------------------------------------------
void calc_collision(collision_aux_t *aux){
    collision_handler_t handler = aux->handler;
    body_t *body1 = aux->body1;
    body_t *body2 = aux->body2;

    collision_info_t info = find_collision(body_get_shape(body1), body_get_shape(body2));
    if (info.collided && !aux->has_collided) {
        handler(body1, body2, info.axis, aux->aux);
        aux->has_collided = true;
    } else if (!info.collided) {
        aux->has_collided = false;
    }

}

void create_collision(
    scene_t *scene,
    body_t *body1,
    body_t *body2,
    collision_handler_t handler,
    void *aux,
    free_func_t freer
) {
    collision_aux_t *aux1 = malloc(sizeof(collision_aux_t));
    assert(aux1 != NULL);
    list_t *bodies = list_init(2, NULL);
    list_add(bodies, body1);
    list_add(bodies, body2);
    aux1->body1 = body1;
    aux1->body2 = body2;
    aux1->handler = handler;
    aux1->aux = aux;
    aux1->freer = freer;
    aux1->has_collided=false;

    scene_add_bodies_force_creator(
        scene,
        (force_creator_t) calc_collision,
        aux1,
        bodies,
        (free_func_t) collision_aux_free
    );
}
void calc_destructive_collision(body_t *body1, body_t *body2, vector_t axis, void *aux) {
    body_remove(body1);
    body_remove(body2);
}

void calc_oneway_destructive_collision(body_t *body1, body_t *body2, vector_t axis, void *aux) {
    body_remove(body2);
}

void calc_damaging_collision(body_t *body1, body_t *body2, vector_t axis, void *aux) {
    double damage = *(double *)body_get_info(body2);
    tank_decrease_health(body1, damage);
    body_remove(body2);
}



void create_destructive_collision(scene_t *scene, body_t *body1, body_t *body2){
    create_collision(scene, body1, body2, (collision_handler_t) calc_destructive_collision,
                     NULL, NULL);
}

// Only deletes the second body
void create_oneway_destructive_collision(scene_t *scene, body_t *body1, body_t *body2){
    create_collision(scene, body1, body2, (collision_handler_t) calc_oneway_destructive_collision,
                     NULL, NULL);
}

//For bullets interacting with tanks
void create_damaging_collision(scene_t *scene, body_t *body1, body_t *body2) {
    create_collision(scene, body1, body2, (collision_handler_t) calc_damaging_collision, NULL, NULL);
}

void calc_physics_collision(body_t *body1, body_t *body2, vector_t axis, physics_aux_t *aux) {
    double elasticity = aux->elasticity;

    double u1 = vec_dot(body_get_velocity(body1), axis);
    double u2 = vec_dot(body_get_velocity(body2), axis);
    double m1 = body_get_mass(body1);
    double m2 = body_get_mass(body2);
    double reduced_mass = 0.0;

    if (m1 >= INFINITY || m2 >= INFINITY) {
        // Uses the minimum of the two masses
        reduced_mass = (m1 < m2) ? m1 : m2;
    } else {
        reduced_mass = (m1 * m2)/(m1 + m2);
    }

    double impulse = reduced_mass*(1+elasticity)*(u2-u1);
    body_add_impulse(body1, vec_multiply(impulse, axis));
    body_add_impulse(body2, vec_multiply(-1*impulse, axis));
}

void create_physics_collision(scene_t *scene,
        double elasticity,
        body_t *body1,
        body_t *body2){
    physics_aux_t *aux = malloc(sizeof(physics_aux_t));
    assert(aux != NULL);

    aux->elasticity = elasticity;

    create_collision(scene, body1, body2, (collision_handler_t) calc_physics_collision,
                     aux, (free_func_t) physics_aux_free);
}


// TANK STUFF

void calc_terrain_follow(terrain_aux_t *aux) {
    body_t *terrain = aux->terrain;
    body_t *tank = aux->body;

    double min_x = 100000;
    double max_x = 0;
    double min_y = 100000;

    list_t *tank_pts = body_get_shape(tank);
    for (size_t i=0; i < list_size(tank_pts); i++) {
        vector_t *pt = (vector_t *) list_get(tank_pts, i);
        if (pt->x < min_x) {
            min_x = pt->x;
        }
        if (pt->x > max_x) {
            max_x = pt->x;
        }
        if (pt->y < min_y) {
            min_y = pt->y;
        }
    }

    vector_t terrain_max = body_get_max(terrain, min_x, max_x);

    vector_t center = body_get_centroid(tank);
    // we want to find the distance from the bottom of the tank and terrain_max
    double height_diff = min_y - terrain_max.y;
    if (fabs(height_diff) > 0.01) {
        body_set_centroid(tank, vec_subtract(center, (vector_t) {0, height_diff - 1}));
    }

}

void create_terrain_follow(scene_t *scene, body_t *terrain, body_t *tank) {
    list_t *bodies = list_init(2, NULL);
    list_add(bodies, terrain);
    list_add(bodies, tank);
    terrain_aux_t *aux = malloc(sizeof(terrain_aux_t));
    aux->terrain = terrain;
    aux->body = tank;
    scene_add_bodies_force_creator(
        scene,
        (force_creator_t) (calc_terrain_follow),
        aux,
        bodies,
        (free_func_t) (free)
    );
}

void calc_health_follow(health_aux_t *aux) {
    body_t *tank = aux->tank;
    update_health_bar(tank);
}

void create_health_follow(scene_t *scene, body_t *tank) {
    list_t *bodies = list_init(1, NULL);
    list_add(bodies, tank);
    health_aux_t *aux = malloc(sizeof(health_aux_t));
    aux->tank = tank;
    scene_add_bodies_force_creator(
        scene,
        (force_creator_t) (calc_health_follow),
        aux,
        bodies,
        (free_func_t) (free)
    );
}

// Bullet Stuff
bool is_colliding(body_t *terrain, body_t *body) {
    double min_x = 100000;
    double max_x = 0;
    double min_y = 100000;

    list_t *bullet_pts = body_get_shape(body);
    for (size_t i=0; i < list_size(bullet_pts); i++) {
        vector_t *pt = (vector_t *) list_get(bullet_pts, i);
        if (pt->x < min_x) {
            min_x = pt->x;
        }
        if (pt->x > max_x) {
            max_x = pt->x;
        }
        if (pt->y < min_y) {
            min_y = pt->y;
        }
    }

    vector_t terrain_max = body_get_max(terrain, min_x, max_x);
    return min_y - terrain_max.y <= 0.0;
}

void calc_collision_with_terrain(collision_terrain_aux_t *aux){
    collision_handler_t handler = aux->handler;
    body_t *body1 = aux->terrain;
    body_t *body2 = aux->body;
    bool colliding = is_colliding(body1, body2);
    if (colliding && !aux->has_collided) {
        handler(body1, body2, VEC_ZERO, aux->aux);
        aux->has_collided = true;
    } else if (!colliding) {
        aux->has_collided = false;
    }

}



void create_terrain_collision(scene_t *scene, body_t *terrain, body_t *bullet, collision_handler_t handler, void *aux, free_func_t freer) {
    list_t *bodies = list_init(2, NULL);
    list_add(bodies, terrain);
    list_add(bodies, bullet);
    collision_terrain_aux_t *aux1 = malloc(sizeof(collision_terrain_aux_t));
    aux1->terrain = terrain;
    aux1->body = bullet;
    aux1->handler = handler;
    aux1->freer = freer;
    aux1->aux = aux;
    scene_add_bodies_force_creator(
        scene,
        (force_creator_t) (calc_collision_with_terrain),
        aux1,
        bodies,
        (free_func_t) (freer)
    );
}

void calc_bullet_terrain_destroy(body_t *body1, body_t *body2, vector_t axis, bullet_aux_t *aux) {
    body_remove(body2);
}

void create_bullet_destroy(scene_t *scene, body_t *terrain, body_t *bullet) {
    create_terrain_collision(scene, terrain, bullet, (collision_handler_t) calc_bullet_terrain_destroy,
        NULL, NULL);
}

void calc_bullet_rotate(rotate_aux_t *aux) {
    body_t *bullet = aux->body;

    vector_t velo = body_get_velocity(bullet);
    double angle;
    if (velo.x < 0) {
        angle = atan(velo.y / velo.x) + PI;
    }
    else {
        angle = atan(velo.y / velo.x);
    }
    body_set_rotation(bullet, angle);
}

void create_bullet_rotate(scene_t *scene, body_t *bullet) {
    list_t *bodies = list_init(1, NULL);
    list_add(bodies, bullet);
    rotate_aux_t *aux = malloc(sizeof(rotate_aux_t));
    aux->body = bullet;
    scene_add_bodies_force_creator(
        scene,
        (force_creator_t) (calc_bullet_rotate),
        aux,
        bodies,
        (free_func_t) (free)
    );
}

void calc_barrel_rotate(barrel_rotate_aux_t *aux) {
    body_t *tank = aux->tank;
    vector_t offset = aux->offset;
    body_t *barrel = tank_get_barrel(tank);

    body_set_centroid(barrel, vec_add(body_get_centroid(tank), offset));
    double angle;
    if (tank_get_number(tank) == 1) {
      angle = PI * tank_get_angle(tank) / 180.0;
    } else {
      angle = PI * tank_get_angle(tank) / 180.0 * -1.0;
    }



    body_set_rotation2(barrel, angle, body_get_centroid(tank));

    // list_t *points = body_get_points(barrel);  // THIS IS MORE ACCURATE, ROTATES AROUND CENTER OF TANK
    // polygon_rotate(points, angle, body_get_centroid(tank));
}

void create_barrel_rotate(scene_t *scene, body_t *tank, vector_t offset) {
    list_t *bodies = list_init(1, NULL);
    list_add(bodies, tank);
    barrel_rotate_aux_t *aux = malloc(sizeof(barrel_rotate_aux_t));
    aux->tank = tank;
    aux->offset = offset;

    scene_add_bodies_force_creator(
        scene,
        (force_creator_t) (calc_barrel_rotate),
        aux,
        bodies,
        (free_func_t) (free)
    );
}
