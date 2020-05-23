# Active-Contours-CLI

## Overview
### Command line interface active contours program
This project utilizes an active contours algorithm with a command line interface (no GUI). The program loads a grayscale PPM image and list of initial contour points. From there, contour points were processed using 2 internal energy terms and 1 external energy term. The window around each contour point was 7x7 pixels. Each energy term was normalized to a 0-1 range and equally weighted. The active contour algorithm was run for 30 iterations total. Once calculated, both the initial contour and final, active contour points were drawn on the original image using a 7x7 black ”+” shapes.

## Discussion

For the first of the internal energy terms, the curvature between the points in the contour was calculated. This was done using the finite, discrete difference below. This internal energy term was used to ensure smoothness among the points in the active contour.

<img src="https://render.githubusercontent.com/render/math?math=E_{internal, 1} = {(x_{i-1} - 2x_i + x_{i+1})}^2 + {(y_{i-1} - 2y_i + y_{i+1})}^2">

For the second of the two internal energy terms, the distance from the centroid of the region enclosed by the contour was calculated. All row and column locations for the contour were averaged together on each iteration to calculate the center. The energy term that tended towards this centroid was calculated with the following:

<img src="https://render.githubusercontent.com/render/math?math=E_{internal, 2} = {(x_i - x_{centroid})}^2 + {(y_i - y_{centroid})}^2">

Lastly, the external energy term used the negative of the image gradient:

<img src="https://render.githubusercontent.com/render/math?math=E_{external} = -||\nabla I(x_i, y_i)||">

From there, the total energy was calculated by summing the 2 internal energy terms and 1
external energy term.

<img src="https://render.githubusercontent.com/render/math?math=E_{total} = E_{internal, 1} + E_{internal, 2} + E_{external}">

The minimum total energy was found for each point on the contour for each iteration. Then, the contour was moved to that point after all of the other points had been calculated for that iteration.

It was found effective to exapnd the window around each contour point from 7x7 pixels to 11x11 pixels for better active contouring for the test case included.

## Usage

`./activecontours [<filename>.ppm] [contour_locations.txt]`

This program only works with grayscale PPM images.

## Test Case


