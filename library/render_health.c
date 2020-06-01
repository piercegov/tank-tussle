#include "render_health.h"
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

body_t *create_bar(vector_t c, double w, double h, rgb_color_t color) {
    list_t *bar = list_init(4, (free_func_t) free);
    list_add(bar, (void *)vec_init(c.x - (w/2.0), c.y + (h/2.0)));
    list_add(bar, (void *)vec_init(c.x - (w/2.0), c.y - (h/2.0)));
    list_add(bar, (void *)vec_init(c.x + (w/2.0), c.y -(h/2.0)));
    list_add(bar, (void *)vec_init(c.x + (w/2.0), c.y + (h/2.0)));
    return body_init(bar, 1.0, color);
}

health_bar_t *health_init(body_t *t1) {
    health_bar_t *hb = malloc(sizeof(health_bar_t));
    vector_t center = body_get_centroid(t1);
    hb->outer = create_bar(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH, HEALTH_HEIGHT, BLACKK);
    hb->inner = create_bar(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH - OUTLINE_WIDTH, HEALTH_HEIGHT - OUTLINE_WIDTH, RED);
    hb->health_pool = create_bar(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH - OUTLINE_WIDTH, HEALTH_HEIGHT - OUTLINE_WIDTH, GREEN);
    return hb;
}

void update_health_bar(body_t *t1) {
    health_bar_t *hb = tank_get_health_bar(t1);
    double health = tank_get_health(t1);
    vector_t center = body_get_centroid(t1);
    double new_width = (HEALTH_WIDTH - OUTLINE_WIDTH) * health;
    // hb->outer = create_bar(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH, HEALTH_HEIGHT, BLACKK);
    // hb->inner = create_bar(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH - OUTLINE_WIDTH, HEALTH_HEIGHT - OUTLINE_WIDTH, RED);
    body_set_centroid(hb->inner, vec_add(center, HEALTH_OFFSET));
    body_set_centroid(hb->outer, vec_add(center, HEALTH_OFFSET));
    body_set_centroid(hb->health_pool, vec_add(center, HEALTH_OFFSET));
    // hb->health_pool = create_bar(vec_add(center, HEALTH_OFFSET), new_width, HEALTH_HEIGHT - OUTLINE_WIDTH, RED);
}
