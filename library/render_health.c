#include "render_health.h"
#include "list.h"
#include "vector.h"

const double OUTLINE_WIDTH = 0.1;
const vector_t HEALTH_OFFSET = {0.0, 5.0};
const double HEALTH_WIDTH = 15.0;
const double HEALTH_HEIGHT = 5.0;
const rgb_color_t WHITE = {1.0, 1.0, 1.0};
const rgb_color_t BLACK = {0.0, 0.0, 0.0};
const rgb_color_t RED = {0.0, 0.0, 0.0};

typedef struct health_bar {
    body_t *outer;
    body_t *inner;
    body_t *health_pool;
} health_bar_t;


body_t *create_bar(vector_t c, double w, double h, rgb_color_t color) {
    list_t *bar = list_init(4, (free_func_t) free);
    list_add(bar, (vector_t){c.x - (w/2.0), c.y + (h/2.0)});
    list_add(bar, (vector_t){c.x + (w/2.0), c.y + (h/2.0)});
    list_add(bar, (vector_t){c.x + (w/2.0), c.y - (h/2.0)});
    list_add(bar, (vector_t){c.x - (w/2.0), c.y - (h/2.0)});
    return body_init(bar, 0.0, color);
}


health_bar_t *health_init(tank_t *t1) {
    health_bar_t *hb = malloc(sizeof(health_bar_t));
    vector_t center = body_get_centroid(tank_get_body(t1));
    hb->outer = create_bar(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH, HEALTH_HEIGHT, WHITE);
    hb->inner = create_bar(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH - OUTLINE_WIDTH, HEALTH_HEIGHT - OUTLINE_WIDTH, BLACK);
    hb->health_pool = create_bar(vec_add(center, HEALTH_OFFSET), HEALTH_WIDTH - OUTLINE_WIDTH, HEALTH_HEIGHT - OUTLINE_WIDTH, RED);
    return hb;
}

void update_health_bar(tank_t *t1) {
    health_bar_t *hb = tank_health_bar(t1);
    double health = tank_health(t1);
    double new_width = (HEALTH_WIDTH - OUTLINE_WIDTH) * health;
    body_free(hb->health_pool);
    hb->health_pool = create_bar(vec_add(TANK_CENTER, HEALTH_OFFSET), new_width, HEALTH_HEIGHT - OUTLINE_WIDTH, RED);
}
