#include "render_health.h"
#include "polygon.h"
#include "list.h"
#include "vector.h"
#include "tank.h"
#include <stdlib.h>

const vector_t HEALTH_OFFSET = {0.0, 15.0};

health_bar_t *health_init(body_t *t1) {
    health_bar_t *hb = malloc(sizeof(health_bar_t));
    vector_t center = body_get_centroid(t1);
    list_t *outer_l = create_rectangle(vec_add(center, HEALTH_OFFSET), BAR_WIDTH, BAR_HEIGHT);
    list_t *inner_l = create_rectangle(vec_add(center, HEALTH_OFFSET), BAR_WIDTH - OUTLINE_WIDTH, BAR_HEIGHT - OUTLINE_WIDTH);
    list_t *health_l = create_rectangle(vec_add(center, HEALTH_OFFSET), BAR_WIDTH - OUTLINE_WIDTH, BAR_HEIGHT - OUTLINE_WIDTH);
    hb->outer = body_init(outer_l, 1.0, BLACK);
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
    double new_width = (BAR_WIDTH - OUTLINE_WIDTH) * (double)health / 100.0;
    // hb->outer = create_bar(vec_add(center, HEALTH_OFFSET), BAR_WIDTH, BAR_HEIGHT, BLACK);
    // hb->inner = create_bar(vec_add(center, HEALTH_OFFSET), BAR_WIDTH - OUTLINE_WIDTH, BAR_HEIGHT - OUTLINE_WIDTH, RED);
    body_set_centroid(hb->inner, vec_add(center, HEALTH_OFFSET));
    body_set_centroid(hb->outer, vec_add(center, HEALTH_OFFSET));
    // body_set_centroid(hb->health_pool, vec_add(center, HEALTH_OFFSET));

    list_t *old_points = body_get_points(hb->health_pool);
    vector_t pool_offset = vec_subtract(vec_add(HEALTH_OFFSET, (vector_t){OUTLINE_WIDTH / 2.0, 0}), (vector_t){(BAR_WIDTH - new_width) / 2.0, 0});
    list_t *new_points = create_rectangle(vec_add(center, pool_offset), new_width, BAR_HEIGHT - OUTLINE_WIDTH);

    for (size_t i = 0; i < list_size(old_points); i++) {
        list_remove(old_points, i);
    }

    for (size_t i = 0; i < list_size(new_points); i++) {
        list_add(old_points, list_get(new_points, i));
    }
    free(new_points);


    // hb->health_pool = create_bar(vec_add(center, HEALTH_OFFSET), new_width, BAR_HEIGHT - OUTLINE_WIDTH, RED);
}
