#include "polygon.h"
#include "power_bar.h"
#include "list.h"
#include "vector.h"
#include "tank.h"
#include <stdlib.h>

power_bar_t *power_bar_init(body_t *t1, vector_t center) {
    power_bar_t *pb = malloc(sizeof(power_bar_t));
    list_t *outer_l = create_rectangle(center, BAR_WIDTH, BAR_HEIGHT);
    list_t *inner_l = create_rectangle(center, BAR_WIDTH - OUTLINE_WIDTH, BAR_HEIGHT - OUTLINE_WIDTH);
    vector_t pool_offset = (vector_t){(BAR_WIDTH - 0.2) / 2.0, 0};

    list_t *power_l = create_rectangle(vec_subtract(center, pool_offset), 0.2, BAR_HEIGHT - OUTLINE_WIDTH);
    pb->outer = body_init(outer_l, 1.0, BLACK);
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

    double new_width = (BAR_WIDTH - OUTLINE_WIDTH) * (double) power / 100.0;
    list_t *old_points = body_get_points(pb->power_level);
    vector_t pool_offset = (vector_t){(BAR_WIDTH - new_width) / 2.0, 0};
    list_t *new_points = create_rectangle(vec_subtract(center, pool_offset), new_width, BAR_HEIGHT - OUTLINE_WIDTH);

    for (size_t i = 0; i < list_size(old_points); i++) {
        free(list_remove(old_points, i));
    }

    for (size_t i = 0; i < list_size(new_points); i++) {
        list_add(old_points, list_get(new_points, i));
    }
    free(list_get_data(new_points));
    free(new_points);

}
