#include "body.h"
#include "terrain.h"
#include <stdlib.h>
#include "list.h"
#include <math.h>
#include "color.h"
#include <time.h>
#include <assert.h>

double interpolate(vector_t left, vector_t right, double x_pos) {
    return (left.y * (right.x - x_pos) + right.y * (x_pos - left.x)) / (right.x - left.x);
}

/* FOR THE FUTURE:
    level 1: num_points = width / 10
    level 2: num_points = width / 5
    level 3: num_points = width / 2.5 (ish) <-- shit gets weird

    PROBLEM: Would need another way of fixing the tanks to the terrain (currently relies on the tank being over vertices)
*/

// Returns a filled/interpolated copy of arr
double *gap_fill(double *arr, size_t width) {
    double *new_lst = malloc(width * sizeof(double));
    assert(new_lst != NULL);
    for (size_t i = 0; i < width; i++) {
        if (arr[i] == 0){ // Has not been changed
            // Find the closest left and right points
            vector_t left = {0, 0};
            for (size_t j=0; j<i; j++) {
                if (height_lst[j] != 0) {
                    left = (vector_t) {j, height_lst[j]};
                }
            }

            vector_t right = {0, 0};
            for (size_t j=i; j<width; j++) {
                if (height_lst[j] != 0) {
                    right = (vector_t) {j, height_lst[j]};
                    break;
                }
            }

            new_lst[i] = interpolate(left, right, i);
        } else {
            new_lst[i] = height_lst[i];
        }
    }

    return new_lst;
}

double *generate_noise(double width, int granularity, double damping) {
  // Assuming width on the order of 100
  // Granularity is the number of levels/octaves to generate
  // 0 < damping < 1

  // Will generate a list of numbers between 0,  1
  double *height_lst = malloc(width * sizeof(double));
  assert(height_lst != NULL);

    for (size_t i=0; i < width; i++) {
        height_lst[i] = 0;
    }
    srand(time(0)); // Set random seed
    for (size_t i=0; i < width; i += 10) {
      height_lst[i] = fmod(rand() / 10.0, 1.0) * 2;
    }

    int level = 2;
    while (level <= granularity) {
      size_t pos = (size_t)(10.0/level); // Rounds down to nearest whole number
      for (size_t i = pos; i < width - pos; i += (size_t) 10.0/(level-1)) { // This will be bad for level > 3
          double l_height = height_lst[i-pos];
          double r_height = height_lst[i+pos];

          double interp = (l_height + r_height)/2;
          double noise = fmod(rand() / 10.0, 1.0) * 2 * pow(damping, level - 1);
          height_lst[i] = interp + noise;
      }
      level += 1;
    }

    double *new_lst = malloc(width * sizeof(double));
    assert(new_lst != NULL);

    // Fill in gaps
    double *new_lst = gap_fill(height_lst, width);

    free(height_lst);
    return new_lst;
}



body_t *generate_terrain(double width, double base_height, double scale, int granularity, double damping, double mass) {
  double* heights = generate_noise(width, granularity, damping);
  // Going to effectively flip across the Y axis
  // So I don't have to going through the list in reverse

  list_t *points = list_init(width + 2, free); // + 2 for the bottom left and right vertices
  for (size_t i=0; i < width; i++) {
      vector_t *pt = malloc(sizeof(vector_t));
      assert(pt != NULL);

      *pt = (vector_t) {i, (heights[i] * scale) + base_height};

      list_add(points, pt);
  }
  free(heights);

  vector_t *l_corner = vec_init(0.0, -100.0);
  vector_t *r_corner = vec_init(width, -100.0);
  list_add(points, r_corner);
  list_add(points, l_corner);


  rgb_color_t color = {0.0, 153.0/255.0, 51.0/255.0};
  body_t* terrain = body_init(points, mass, color);

  return terrain;
}
