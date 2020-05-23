
/*
** Adam Patyk
** ECE 6310
** Lab 5: Active Contours
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

unsigned char *read_image(char **, int, int *, int *, int *);
float *normalize(float *, int, int, int);
unsigned char* sobel_filter(unsigned char *, int, int);
unsigned char* draw_contour(unsigned char *, int, int, int **, int);
int** process_contour(unsigned char *, int, int, int **, int, int);

int main(int argc, char *argv[]){
    FILE		    *fpt;
    unsigned char	*input_img, *sobel_img, *init_contour, *final_contour;
    int		        i, rows, cols, bytes, tmp1, tmp2, num_points = 0;
    int             **active_contour;

    if (argc != 3) {
      printf("Usage: ./activecontours [<filename>.ppm] [contour_locations.txt]\n");
      exit(0);
    }

    // read input image
    input_img = read_image(argv, 1, &rows, &cols, &bytes);

    // read contour points file
    if ((fpt = fopen(argv[2], "rb")) == NULL) {
        printf("Unable to open \"%s\" for reading\n", argv[2]);
        exit(0);
    }
    // determine length of file
    while (fscanf(fpt, "%d %d\n", &tmp1, &tmp2) != EOF) {
        num_points++;
    }
    fseek(fpt, 0, SEEK_SET); // reset file pointer to beginning of file
    int **contour_coords = (int **)calloc(num_points, sizeof(int *));
    for (i = 0; i < num_points; i++) {
        contour_coords[i] = (int *)calloc(2, sizeof(int));
    }
    // read values into memory
    for (i = 0; i < num_points; i++) {
        fscanf(fpt, "%d %d\n", &contour_coords[i][0], &contour_coords[i][1]);
    }
    fclose(fpt);

    // perform Sobel filtering
    sobel_img = sobel_filter(input_img, rows, cols);
    // draw initial contour on image
    init_contour = draw_contour(input_img, rows, cols, contour_coords, num_points);
    // process active contour
    active_contour = process_contour(sobel_img, rows, cols, contour_coords, num_points, 7);
    // draw final contour on image
    final_contour = draw_contour(input_img, rows, cols, contour_coords, num_points);

    // write out Sobel filter image
    fpt = fopen("sobel_filter_img.ppm", "w");
    fprintf(fpt, "P5 %d %d 255\n", cols, rows);
    fwrite(sobel_img, cols * rows, 1, fpt);
    fclose(fpt);
    // write out initial contour image
    fpt = fopen("init_contour_img.ppm", "w");
    fprintf(fpt, "P5 %d %d 255\n", cols, rows);
    fwrite(init_contour, cols * rows, 1, fpt);
    fclose(fpt);
    // write out final contour image
    fpt = fopen("final_contour_img.ppm", "w");
    fprintf(fpt, "P5 %d %d 255\n", cols, rows);
    fwrite(final_contour, cols * rows, 1, fpt);
    fclose(fpt);
    // write out final active contour coordinates
    fpt = fopen("active_contour.txt", "w");
    for (i = 0; i < num_points; i++) {
        fprintf(fpt, "%d %d\n", active_contour[i][0], active_contour[i][1]);
    }
    fclose(fpt);

    free(input_img);
    free(sobel_img);
    free(init_contour);
    free(final_contour);
    for (i = 0; i < num_points; i++)
         free(active_contour[i]);
    free(active_contour);

    return 0;
}

unsigned char *read_image(char **argv, int arg, int *rows, int *cols, int *bytes) {
    FILE *fpt;
    char header[320];

    if ((fpt = fopen(argv[arg], "rb")) == NULL) {
        printf("Unable to open \"%s\" for reading\n", argv[arg]);
        exit(0);
    }
    fscanf(fpt, "%s %d %d %d", header, cols, rows, bytes);
    if (strcmp(header,"P5") != 0  ||  *bytes != 255) {
        printf("Not a grayscale 8-bit PPM image\n");
        exit(0);
    }
    unsigned char *img = (unsigned char *)calloc(*rows * *cols, sizeof(unsigned char));
    header[0] = fgetc(fpt);	// read white-space character that separates header
    fread(img, 1, *cols * *rows, fpt);
    fclose(fpt);

    return img;
}

unsigned char* sobel_filter(unsigned char *input_img, int rows, int cols) {
    int i, r, c, r2, c2, val, max = 0, 
    f1[] = {    -1,   0,    1,
                -2,   0,    2,
                -1,   0,    1,  },
    f2[] = {    -1,  -2,   -1,
                 0,   0,    0,
                 1,   2,    1,  };
    float val_x, val_y;
    int	*tmp_img = (int *)calloc(rows * cols, sizeof(int));
    unsigned char *output_img = (unsigned char *)calloc(rows * cols, sizeof(unsigned char));
    
    for (r = 1; r < rows - 1; r++) {
        for (c = 1; c < cols - 1; c++) {
            i = val_x = val_y = 0;
            for (r2 = -1; r2 <= 1; r2++) {
                for (c2 = -1; c2 <= 1; c2++, i++) {
                    /* filter horizontally */
                    val_x += input_img[(r + r2) * cols + (c + c2)] * f1[i];
                    /* filter vertically */
                    val_y += input_img[(r + r2) * cols + (c + c2)] * f2[i];
                }
            }
            val = (int)sqrt(pow(val_x, 2) + pow(val_y, 2));
            tmp_img[r * cols + c] = val;
        }
    }

    // normalize
    for (r = 1; r < rows - 1; r++) {
        for (c = 1; c < cols - 1; c++) {
            // calculate the max to find the normalization factor
            if (tmp_img[r * cols + c] > max) {
                max = tmp_img[r * cols + c];
            }
        }
    }
    for (r = 1; r < rows - 1; r++) {
        for (c = 1; c < cols - 1; c++) {
            // apply offset and normalize to 8 bits
            output_img[r * cols + c] = (unsigned char)((float)tmp_img[r * cols + c] / max * 255);
        }
    }

    return output_img;
}

float *normalize(float *input, int rows, int cols, int max_val) {
    int r, c, max, min, offset, range;
    max = min = input[0];
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            // calculate the min to find the low bound
            if (input[r * cols + c] < min) {
                min = input[r * cols + c];
            }
            // calculate the max to find the normalization factor
            if (input[r * cols + c] > max) {
                max = input[r * cols + c];
            }
        }
    }
    offset = abs(min);
    range = offset + max;
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            // apply offset and normalize to 8 bits
            input[r * cols + c] = input[r * cols + c] + offset;
            input[r * cols + c] = input[r * cols + c] / range * max_val;
        }
    }
    return input;
}

unsigned char* draw_contour(unsigned char *img, int rows, int cols, int **contour_pts, int num_points) {
    int i, r, c, r2, c2;

    unsigned char *output = (unsigned char *)calloc(rows * cols, sizeof(unsigned char));

    // copy image
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            output[r * cols + c] = img[r * cols + c];
        }
    }

    for (i = 0; i < num_points; i++) {
        r = contour_pts[i][1];
        c = contour_pts[i][0];
        if (r < 3 || c < 3 || r > rows - 3 || c > cols - 3)
            continue;
        // draw X at contour point locations
        for (r2 = -3; r2 <= 3; r2++) {
            output[(r + r2) * cols + c] = 0;
        }
        for (c2 = -3; c2 <= 3; c2++) {
            output[r * cols + (c + c2)] = 0;
        }
    }

    return output;
}

int** process_contour(unsigned char *sobel, int rows, int cols, int **contour_pts, int num_points, int window_size) {
    int i, j, n, r, c, r2, c2;
    int r_sum, c_sum, r_center, c_center, min_loc, r_tr, c_tr;
    int last, x_next, x_curr, x_prev, y_next, y_curr, y_prev;
    float min;
    int **active_contour_pts = (int **)calloc(num_points, sizeof(int *));
    for (i = 0; i < num_points; i++) {
        active_contour_pts[i] = (int *)calloc(2, sizeof(int));
    }
    float	*int_energy1 = (float *)calloc(pow(window_size, 2), sizeof(float));
    float	*int_energy2 = (float *)calloc(pow(window_size, 2), sizeof(float));
    float	*ext_energy = (float *)calloc(pow(window_size, 2), sizeof(float));
    float	*tot_energy = (float *)calloc(pow(window_size, 2), sizeof(float));

    for (i = 0; i < 30; i++) {
        // find centroid
        r_sum = c_sum = 0;
        for (n = 0; n < num_points; n++) {
            r_sum += contour_pts[n][1];
            c_sum += contour_pts[n][0];
        }
        r_center = r_sum/num_points;
        c_center = c_sum/num_points;

        for (n = 0; n < num_points; n++) {
            r = contour_pts[n][1];
            c = contour_pts[n][0];
            j = 0;

            // calculate internal energy term (discrete curvature)
            for (r2 = -window_size/2; r2 <= window_size/2; r2++) {
                for (c2 = -window_size/2; c2 <= window_size/2; c2++, j++) {
                    last = num_points - 1;
                    // wrap at ends of list
                    x_next = (n == last) ? contour_pts[0][0] : contour_pts[n + 1][0];
                    x_curr = c + c2;
                    x_prev = (n == 0) ? contour_pts[last][0] : contour_pts[n - 1][0];

                    y_next = (n == last) ? contour_pts[0][1] : contour_pts[n + 1][1];
                    y_curr = r + r2;
                    y_prev = (n == 0) ? contour_pts[last][1] : contour_pts[n - 1][1];

                    int_energy1[j] = pow(x_prev - 2 * x_curr + x_next, 2) + pow(y_prev - 2 * y_curr + y_next, 2);
                }
            }
            // normalize internal energy 
            int_energy1 = normalize(int_energy1, window_size, window_size, 1);

            // calculate internal energy term (center)
            j = 0;
            for (r2 = -window_size/2; r2 <= window_size/2; r2++) {
                for (c2 = -window_size/2; c2 <= window_size/2; c2++, j++) {
                    int_energy2[j] = pow(r + r2 - r_center, 2) + pow(c + c2 - c_center, 2);
                }
            }
            // normalize internal energy 
            int_energy2 = normalize(int_energy2, window_size, window_size, 1);

            // calculate external energy term
            j = 0;
            for (r2 = -window_size/2; r2 <= window_size/2; r2++) {
                for (c2 = -window_size/2; c2 <= window_size/2; c2++, j++) {
                    ext_energy[j] = -sobel[(r + r2) * cols + (c + c2)];
                }
            }
            // normalize external energy 
            ext_energy = normalize(ext_energy, window_size, window_size, 1);

            // calculate total energy
            for (j = 0; j < pow(window_size, 2); j++) {
                tot_energy[j] = int_energy1[j] + int_energy2[j] + ext_energy[j];
            }

            // find minimum total energy
            min = tot_energy[0];
            min_loc = 0;
            for (j = 0; j < pow(window_size, 2); j++) {
                if (tot_energy[j] < min) {
                    min = tot_energy[j];
                    min_loc = j;
                }
                // in case of tie, pick points closer to center
                if (tot_energy[j] == min) {
                    r_tr = j / window_size - (window_size / 2);
                    c_tr = j % window_size - (window_size / 2);

                    if (abs(contour_pts[n][0] + c_tr - c_center) < abs(contour_pts[n][0] - c_center) &&
                        abs(contour_pts[n][1] + r_tr - r_center) < abs(contour_pts[n][1] - r_center)) {
                            min = tot_energy[j];
                            min_loc = j;
                    }
                }
            }
            r_tr = min_loc / window_size - (window_size / 2);
            c_tr = min_loc % window_size - (window_size / 2);
        
            active_contour_pts[n][0] = contour_pts[n][0] + c_tr;
            active_contour_pts[n][1] = contour_pts[n][1] + r_tr;
        }
        // move contour points
        for (n = 0; n < num_points; n++) {
            contour_pts[n][0] = active_contour_pts[n][0];
            contour_pts[n][1] = active_contour_pts[n][1];
        }
    }

    return active_contour_pts;
}