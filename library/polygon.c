#include "polygon.h"
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

const double SIX = 6;
const double CIRC_DENSITY = 40.0;
const double PI = 3.14159265;


list_t *create_rectangle(vector_t c, double w, double h) {
    list_t *bar = list_init(4, (free_func_t) free);
    list_add(bar, (void *)vec_init(c.x - (w/2.0), c.y + (h/2.0)));
    list_add(bar, (void *)vec_init(c.x - (w/2.0), c.y - (h/2.0)));
    list_add(bar, (void *)vec_init(c.x + (w/2.0), c.y -(h/2.0)));
    list_add(bar, (void *)vec_init(c.x + (w/2.0), c.y + (h/2.0)));
    return bar;
}

list_t *create_arc(double d, double rads) {
    double total_pts = CIRC_DENSITY * rads;
    list_t *points = list_init(total_pts, (free_func_t) free);
    for (size_t i = 0; i < total_pts; i++) {
        vector_t *new_pt = malloc(sizeof(vector_t));
        *new_pt = vec_rotate((vector_t) {d / 2, 0}, i / CIRC_DENSITY);
        list_add(points, new_pt);
    }
    return points;
}

double polygon_area(list_t *polygon) {
  double area = 0.0;
  size_t size = list_size(polygon);
  size_t i;
  for (i = 0; i < size - 1; i++) {
    area += vec_cross(*(vector_t*)list_get(polygon, i), *(vector_t*)list_get(polygon, i + 1));
  }
  area += vec_cross(*(vector_t*)list_get(polygon, size - 1), *(vector_t*)list_get(polygon, 0));
  return area * THIS_IS_A_HALF;
}

vector_t polygon_centroid(list_t *polygon) {
  double center_x = 0.0;
  double center_y = 0.0;
  double area = polygon_area(polygon);
  size_t size = list_size(polygon);
  size_t i;
  for (i = 0; i < size - 1; i++) {
    vector_t *v1 = list_get(polygon, i);
    vector_t *v2 = list_get(polygon, i + 1);
    center_x += (v1->x + v2->x) * vec_cross(*v1, *v2);
    center_y += (v1->y + v2->y) * vec_cross(*v1, *v2);
  }
  vector_t *v_n = list_get(polygon, size - 1);
  vector_t *v_0 = list_get(polygon, 0);
  center_x += (v_n->x + v_0->x) * vec_cross(*v_n, *v_0);
  center_y += (v_n->y + v_0->y) * vec_cross(*v_n, *v_0);
  vector_t centroid = { center_x / (SIX * area), center_y / (SIX * area) };

  return centroid;
}

void polygon_translate(list_t *polygon, vector_t translation) {
  size_t i;
  size_t size = list_size(polygon);
  for (i = 0; i < size; i++) {
    vector_t *v = list_get(polygon, i);
    *v = vec_add(*v, translation);
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  size_t i;
  size_t size = list_size(polygon);
  for (i = 0; i < size; i++) {
    vector_t *v = list_get(polygon, i);
    *v = vec_subtract(*v, point);
    *v = vec_add(point, vec_rotate(*v, angle));
  }
}
