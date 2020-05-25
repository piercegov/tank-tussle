#include <stdbool.h>

typedef struct interval {
    double min;
    double max;
} interval_t;

bool interval_overlapping(interval_t i1, interval_t i2);

interval_t interval_get_overlap(interval_t i1, interval_t i2);

double interval_get_length(interval_t interval);
