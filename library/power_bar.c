#include "polygon.h"
#include "power_bar.h"
#include "list.h"
#include "vector.h"
#include "tank.h"
#include <stdlib.h>

const double OUTLINE_WIDTHH = 0.5;
const double POWER_WIDTH = 15.0;
const double POWER_HEIGHT = 2.0;
const rgb_color_t WHITE = {1.0, 1.0, 1.0};
const rgb_color_t BLAC = {0.0, 0.0, 0.0};
const rgb_color_t BLUE = {0.0, 0.0, 1.0};

power_bar_t *power_bar_init(body_t *t1, vector_t center) {
    power_bar_t *pb = malloc(sizeof(power_bar_t));
    list_t *outer_l = create_rectangle(center, POWER_WIDTH, POWER_HEIGHT);
    list_t *inner_l = create_rectangle(center, POWER_WIDTH - OUTLINE_WIDTHH, POWER_HEIGHT - OUTLINE_WIDTHH);
    list_t *power_l = create_rectangle(center, 0.1, POWER_HEIGHT - OUTLINE_WIDTHH);
    pb->outer = body_init(outer_l, 1.0, BLAC);
    pb->inner = body_init(inner_l, 1.0, WHITE);
    pb->power_level = body_init(power_l, 1.0, BLUE);
    return pb;
}

void update_power_bar(body_t *t1) {
    power_bar_t *pb = tank_get_power_bar(t1);
    double power = tank_get_power(t1);
    vector_t center = body_get_centroid(pb->outer);

    if (power < 0) {
        power = 0;
    }

    double new_width = (POWER_WIDTH - OUTLINE_WIDTHH) * (double) power / 100.0;
    // hb->outer = create_bar(vec_add(center, POWER_OFFSET), POWER_WIDTH, POWER_HEIGHT, BLACKK);
    // hb->inner = create_bar(vec_add(center, POWER_OFFSET), POWER_WIDTH - OUTLINE_WIDTH, POWER_HEIGHT - OUTLINE_WIDTH, RED);
    // body_set_centroid(pb->inner, vec_add(center, POWER_OFFSET));
    // body_set_centroid(pb->outer, vec_add(center, POWER_OFFSET));
    // body_set_centroid(hb->health_pool, vec_add(center, POWER_OFFSET));

    list_t *old_points = body_get_points(pb->power_level);
    vector_t pool_offset = (vector_t){(POWER_WIDTH - new_width) / 2.0, 0};
    list_t *new_points = create_rectangle(vec_subtract(center, pool_offset), new_width, POWER_HEIGHT - OUTLINE_WIDTHH);

    for (size_t i = 0; i < list_size(old_points); i++) {
        list_remove(old_points, i);
    }

    for (size_t i = 0; i < list_size(new_points); i++) {
        list_add(old_points, list_get(new_points, i));
    }
    free(new_points);

}
