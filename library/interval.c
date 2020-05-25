#include "interval.h"
#include <assert.h>

bool interval_overlapping(interval_t i1, interval_t i2) {
    if (i2.min <= i1.max && i2.max >= i1.max) {
        return true;
    }
    if (i2.max >= i1.min && i2.max <= i1.max) {
        return true;
    }
    if (i2.min >= i1.min && i2.max <= i1.max) {
        return true;
    }
    if (i1.min >= i2.min && i1.max <= i2.max) {
        return true;
    }

    return false;
}

// Returns [0, 0] if they are not overlapping
interval_t interval_get_overlap(interval_t i1, interval_t i2) {
    if (!interval_overlapping(i1, i2)) {
        return (interval_t) {0, 0};
    }

    double start = i1.min;
    double end = i1.max;

    if (i1.min < i2.min) {
        start = i2.min;
    }
    if (i1.max > i2.max) {
        end = i2.max;
    }

    return (interval_t) {start, end};
}

double interval_get_length(interval_t interval) {
    // assert(interval.max >= interval.min);
    return (interval.max - interval.min);
}
