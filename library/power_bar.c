#include "polygon.h"
#include "power_bar.h"
#include "list.h"
#include "vector.h"
#include "tank.h"
#include <stdlib.h>

const double OUTLINE_WIDTH = 0.5;
const vector_t HEALTH_OFFSET = {0.0, 15.0};
const double HEALTH_WIDTH = 15.0;
const double HEALTH_HEIGHT = 2.0;
const rgb_color_t RED = {1.0, 0.0, 0.0};
const rgb_color_t BLACKK = {0.0, 0.0, 0.0};
const rgb_color_t GREEN = {0.0, 1.0, 0.0};


health_bar_t *health_init(body_t *t1) {
    health_bar_t *hb = malloc(sizeof(health_bar_t));
    vector_t center = body_get_centroid(t1);
    list_t *outer_l = create_rectangle(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH, HEALTH_HEIGHT);
    list_t *inner_l = create_rectangle(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH - OUTLINE_WIDTH, HEALTH_HEIGHT - OUTLINE_WIDTH);
    list_t *health_l = create_rectangle(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH - OUTLINE_WIDTH, HEALTH_HEIGHT - OUTLINE_WIDTH);
    hb->outer = body_init(outer_l, 1.0, BLACKK);
    hb->inner = body_init(inner_l, 1.0, RED);
    hb->health_pool = body_init(health_l, 1.0, GREEN);
    return hb;
}

void update_health_bar(body_t *t1) {
    health_bar_t *hb = tank_get_health_bar(t1);
    double health = tank_get_health(t1);
    vector_t center = body_get_centroid(t1);

    if (health < 0) {
        health = 0;
    }
    double new_width = (HEALTH_WIDTH - OUTLINE_WIDTH) * (double)health / 100.0;
    // hb->outer = create_bar(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH, HEALTH_HEIGHT, BLACKK);
    // hb->inner = create_bar(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH - OUTLINE_WIDTH, HEALTH_HEIGHT - OUTLINE_WIDTH, RED);
    body_set_centroid(hb->inner, vec_add(center, HEALTH_OFFSET));
    body_set_centroid(hb->outer, vec_add(center, HEALTH_OFFSET));
    // body_set_centroid(hb->health_pool, vec_add(center, HEALTH_OFFSET));

    list_t *old_points = body_get_points(hb->health_pool);
    vector_t pool_offset = vec_subtract(HEALTH_OFFSET, (vector_t){(HEALTH_WIDTH - new_width) / 2.0, 0});
    list_t *new_points = create_rectangle(vec_add(center, pool_offset), new_width, HEALTH_HEIGHT - OUTLINE_WIDTH);

    for (size_t i = 0; i < list_size(old_points); i++) {
        list_remove(old_points, i);
    }

    for (size_t i = 0; i < list_size(new_points); i++) {
        list_add(old_points, list_get(new_points, i));
    }
    free(new_points);


    // hb->health_pool = create_bar(vec_add(center, HEALTH_OFFSET), new_width, HEALTH_HEIGHT - OUTLINE_WIDTH, RED);
}
