#include "polygon.h"
#include "fuel_bar.h"
#include "list.h"
#include "vector.h"
#include "tank.h"
#include <stdlib.h>

const double OUTLINE_WIDTHHH = 0.5;
const double FUEL_WIDTH = 15.0;
const double FUEL_HEIGHT = 2.0;
const rgb_color_t WHITEE = {1.0, 1.0, 1.0};
const rgb_color_t BLA = {0.0, 0.0, 0.0};
const rgb_color_t YELLOW = {241.0 / 255.0, 185.0 / 255.0, 48.0 / 255.0};

fuel_bar_t *fuel_bar_init(body_t *t1, vector_t center) {
    fuel_bar_t *pb = malloc(sizeof(fuel_bar_t));
    list_t *outer_l = create_rectangle(center, FUEL_WIDTH, FUEL_HEIGHT);
    list_t *inner_l = create_rectangle(center, FUEL_WIDTH - OUTLINE_WIDTHHH, FUEL_HEIGHT - OUTLINE_WIDTHHH);
    list_t *fuel_l = create_rectangle(center, FUEL_WIDTH - OUTLINE_WIDTHHH, FUEL_HEIGHT - OUTLINE_WIDTHHH);
    pb->outer = body_init(outer_l, 1.0, BLA);
    pb->inner = body_init(inner_l, 1.0, WHITEE);
    pb->fuel_level = body_init(fuel_l, 1.0, YELLOW);
    return pb;
}

void update_fuel_bar(body_t *t1) {
    fuel_bar_t *pb = tank_get_fuel_bar(t1);
    double fuel = tank_get_fuel(t1);
    vector_t center = body_get_centroid(pb->outer);

    if (fuel < 0) {
        fuel = 0;
    }

    double new_width = (FUEL_WIDTH - OUTLINE_WIDTHHH) * (double) fuel / 100.0;
    // hb->outer = create_bar(vec_add(center, FUEL_OFFSET), FUEL_WIDTH, FUEL_HEIGHT, BLACKK);
    // hb->inner = create_bar(vec_add(center, FUEL_OFFSET), FUEL_WIDTH - OUTLINE_WIDTH, FUEL_HEIGHT - OUTLINE_WIDTH, RED);
    // body_set_centroid(pb->inner, vec_add(center, FUEL_OFFSET));
    // body_set_centroid(pb->outer, vec_add(center, FUEL_OFFSET));
    // body_set_centroid(hb->health_pool, vec_add(center, FUEL_OFFSET));

    list_t *old_points = body_get_points(pb->fuel_level);
    vector_t pool_offset = (vector_t){(FUEL_WIDTH - new_width) / 2.0, 0};
    list_t *new_points = create_rectangle(vec_subtract(center, pool_offset), new_width, FUEL_HEIGHT - OUTLINE_WIDTHHH);

    for (size_t i = 0; i < list_size(old_points); i++) {
        list_remove(old_points, i);
    }

    for (size_t i = 0; i < list_size(new_points); i++) {
        list_add(old_points, list_get(new_points, i));
    }
    free(new_points);

}
