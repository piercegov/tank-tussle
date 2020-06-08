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

// Returns a filled/interpolated copy of arr
double *gap_fill(double *arr, size_t width) {
    double *new_lst = malloc(width * sizeof(double));
    assert(new_lst != NULL);
    for (size_t i = 0; i < width; i++) {
        if (arr[i] == 0){ // Has not been changed
            // Find the closest left and right points
            vector_t left = {0, 0};
            for (size_t j=0; j<i; j++) {
                if (arr[j] != 0) {
                    left = (vector_t) {j, arr[j]};
                }
            }

            vector_t right = {0, 0};
            for (size_t j=i; j<width; j++) {
                if (arr[j] != 0) {
                    right = (vector_t) {j, arr[j]};
                    break;
                }
            }

            new_lst[i] = interpolate(left, right, i);
        } else {
            new_lst[i] = arr[i];
        }
    }

    return new_lst;
}

void amplify_heights(double *heights, double width, double a) {
    for (size_t i=0; i < width; i++) {
        double amp = (15 * pow(a, 3)) / (pow(0.1 * (i - width/2), 2) + 4 * pow(a, 2)) + 1; // peak at 0
        heights[i] *= amp;
    }
}

double *generate_noise(double width, int granularity, double damping, double a) {
  // Assuming width on the order of 100
  // Granularity is the number of levels/octaves to generate
  // 0 < damping < 1

  // Will generate a list of numbers between -1,  1
  double *height_lst = malloc(width * sizeof(double));
  assert(height_lst != NULL);

    for (size_t i=0; i < width; i++) {
        height_lst[i] = 0;
    }
    srand(time(0)); // Set random seed
    for (size_t i=0; i < width; i += 10) {
      height_lst[i] = (fmod(rand() / 10.0, 1.0) - 0.5) * 2;
    }

    int level = 2;
    while (level <= granularity) {
      size_t pos = (size_t)(10.0/level); // Rounds down to nearest whole number
      for (size_t i = pos; i < width - pos; i += (size_t) 10.0/(level-1)) { // This will be bad for level > 3
          double l_height = height_lst[i-pos];
          double r_height = height_lst[i+pos];

          double interp = (l_height + r_height)/2;
          double noise = (fmod(rand() / 10.0, 1.0) - 0.5) * 2 * pow(damping, level - 1);
          height_lst[i] = interp + noise;
      }
      level += 1;
    }


    // Fill in gaps
    double *new_lst = gap_fill(height_lst, width);

    free(height_lst);

    // Amplify middle points
    amplify_heights(new_lst, width, a);

    return new_lst;
}



body_t *generate_terrain(double width, double base_height, double scale, int granularity, double damping, double mass, double a) {
    double new_width = width + 10; // Ensures that it is wide enough
    double* heights = generate_noise(new_width, granularity, damping, a);
    // Going to effectively flip across the Y axis
    // So I don't have to going through the list in reverse

    list_t *points = list_init(new_width + 2, free); // + 2 for the bottom left and right vertices
    for (size_t i=0; i < new_width; i++) {
        vector_t *pt = malloc(sizeof(vector_t));
        assert(pt != NULL);

        *pt = (vector_t) {i, (heights[i] * scale) + base_height};

        list_add(points, pt);
    }
    free(heights);

    vector_t *l_corner = vec_init(0.0, -100.0);
    vector_t *r_corner = vec_init(new_width, -100.0);
    list_add(points, r_corner);
    list_add(points, l_corner);


    rgb_color_t color = {0.0, 153.0/255.0, 51.0/255.0};
    body_t* terrain = body_init(points, mass, color);

    return terrain;
}
