#include "body.h"
#include "terrain.h"
#include <stdlib.h>
#include "list.h"
#include <math.h>
#include "color.h"

body_t *generate_terrain(double width, double base_height, double scale, int granularity, double damping, double mass) {
    double* heights = generate_noise(width, granularity, damping);
    // Going to effectively flip across the Y axis
    // So I don't have to going through the list in reverse

    list_t points = list_init(width + 2); // + 2 for the bottom left and right vertices
    for (size_t i=0; i < width; i++) {
        vector_t *pt = malloc(sizeof(vector_t));
        assert(pt != NULL);

        *pt = (vector_t) {i, (heights[i] * scale) + base_height}

        list_add(points, pt);
    }
    free(heights);

    vector_t *l_corner = vec_init(0.0, -100.0);
    vector_t *r_corner = vec_init(width, -100.0);

    list_add(points, l_corner);
    list_add(points, r_corner);

    rgb_color_t color = {0.0, 153.0/255.0, 51.0/255.0};
    body_t* terrain = body_init(points, mass, rgb_color_t color)

    return terrain;
}

double *generate_noise(double width, int granularity, double damping) {
    // Assuming width on the order of 100
    // Granularity is the number of levels/octaves to generate
    // 0 < damping < 1

    // Will generate a list of numbers between -1,  1
    double *height_lst = malloc(width * sizeof(double));
    assert(height_lst != NULL);

    srand(time(0)); // Set random seed
    for (size_t i=0; i < width; i += 10) {
        height_lst[i] = (fmod(rand(), 1.0) - 0.5) * 2;
    }

    int level = 2;
    while (level < granularity) {
        for (size_t i = 10.0/level; i < width; i += 10.0/level) {
            double l_height = height_lst[i-2.5];
            double r_height = height_lst[i+2.5];

            double interp = (l_height + r_height)/2
            double noise = (fmod(rand(), 1.0) - 0.5) * 2 * pow(damping, level - 1);
            height_lst[i - fmod(i, 1)] = interp + noise;
        }
    }

    // Fill in gaps
    for (size_t i = 0; i < width; i++) {
        if (height_lst[i] == 0){ // Likely has not been changed
            // Find the closest left and right points
            double l_height = 0;
            for (size_t j=0; j<i; j++) {
                if (height_lst[j] != 0) {
                    l_height = height_lst[j];
                }
            }

            double r_height = 0;
            for (size_t j=i; j<width; j++) {
                if (height_lst[j] != 0) {
                    r_height = height_lst[j];
                }
            }

            height_lst[i] = (l_height + r_height) / 2; // Not completely right but close enough for now
        }
    }

    return height_lst;
}
