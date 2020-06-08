#include "polygon.h"
#include "fuel_bar.h"
#include "list.h"
#include "vector.h"
#include "tank.h"
#include <stdlib.h>

const rgb_color_t YELLOW = {241.0 / 255.0, 185.0 / 255.0, 48.0 / 255.0};

fuel_bar_t *fuel_bar_init(body_t *t1, vector_t center) {
    fuel_bar_t *pb = malloc(sizeof(fuel_bar_t));
    list_t *outer_l = create_rectangle(center, BAR_WIDTH, BAR_HEIGHT);
    list_t *inner_l = create_rectangle(center, BAR_WIDTH - OUTLINE_WIDTH, BAR_HEIGHT - OUTLINE_WIDTH);
    list_t *fuel_l = create_rectangle(center, BAR_WIDTH - OUTLINE_WIDTH, BAR_HEIGHT - OUTLINE_WIDTH);
    pb->outer = body_init(outer_l, 1.0, BLACK);
    pb->inner = body_init(inner_l, 1.0, WHITE);
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

    double new_width = (BAR_WIDTH - OUTLINE_WIDTH) * (double) fuel / 100.0;

    list_t *old_points = body_get_points(pb->fuel_level);
    vector_t pool_offset = (vector_t){(BAR_WIDTH - new_width) / 2.0, 0};
    list_t *new_points = create_rectangle(vec_subtract(center, pool_offset), new_width, BAR_HEIGHT - OUTLINE_WIDTH);

    for (size_t i = 0; i < list_size(old_points); i++) {
        list_remove(old_points, i);
    }

    for (size_t i = 0; i < list_size(new_points); i++) {
        list_add(old_points, list_get(new_points, i));
    }
    free(new_points);

}
