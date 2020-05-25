#include "collision.h"
#include "vector.h"
#include "interval.h"
#include "list.h"
#include "body.h"

// Should project each point in body onto the vector and find the interval of
// the body onto the vector
interval_t list_project(list_t *list, vector_t vector) {
    size_t num_points = list_size(list);
    interval_t interval = (interval_t) {9e10, -9e10};

    for (size_t i = 0; i < num_points; i++) {
        vector_t *point = (vector_t *) list_get(list, i);
        double projection = vec_dot(*point, vector);
        if (projection < interval.min) {
            interval.min = projection;
        }
        if (projection > interval.max) {
            interval.max = projection;
        }
    }

    return interval;
}

vector_t check_edges(size_t num_points, list_t *l1, list_t *l2) {
    vector_t min_vector = {0, 0};
    double min_overlap = 9e10;
    for (size_t i = 0; i < num_points; i++) {
        vector_t *v1 = list_get(l1, i);
        vector_t *v2 = list_get(l1, (i+1)%num_points);
        vector_t edge = (vector_t) {v2->x - v1->x, v2->y - v1->y};

        // Right normal
        vector_t normal = {edge.y, -1 * edge.x};
        double norm = vec_norm(normal);
        normal = vec_multiply(1/norm, normal);

        // Project all vertices of the first body onto the edge
        interval_t interval_1 = list_project(l1, normal);

        interval_t interval_2 = list_project(l2, normal);
        // printf("%f\n", interval_1.min);
        // printf("%f\n", interval_2.max);

        interval_t overlap = interval_get_overlap(interval_1, interval_2);
        double length = interval_get_length(overlap);

        if (length == 0) {
            return (vector_t) {0.0, 0.0};
        } else if (length < min_overlap) {
            min_overlap = length;
            min_vector = normal;
        }
    }

    return min_vector;
}

double get_projection_overlap(vector_t edge, list_t *l1, list_t *l2) {
    interval_t interval_1 = list_project(l1, edge);
    interval_t interval_2 = list_project(l2, edge);
    return interval_get_length(interval_get_overlap(interval_1, interval_2));
}

collision_info_t find_collision(list_t *l1, list_t *l2) {
    size_t num_points_l1 = list_size(l1);
    size_t num_points_l2 = list_size(l2);

    // Separating Axis Theorem
    vector_t min_edge_1 = check_edges(num_points_l1, l1, l2);
    vector_t min_edge_2 = check_edges(num_points_l2, l2, l1);

    // If either of these are NULL, then they are not colliding
    if (!vec_isequal(min_edge_1, VEC_ZERO) && !vec_isequal(min_edge_2, VEC_ZERO)) {
        double overlap_1 = get_projection_overlap(min_edge_1, l1, l2);
        double overlap_2 = get_projection_overlap(min_edge_2, l1, l2);

        if (overlap_1 < overlap_2) {
            return (collision_info_t) {true, min_edge_1};
        } else {
            return (collision_info_t) {true, min_edge_2};
        }

    } else {
        return (collision_info_t) {false, (vector_t) {0, 0}};
    }
}
