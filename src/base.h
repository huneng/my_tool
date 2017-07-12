#ifndef _HU_TOOL_BASE_H_
#define _HU_TOOL_BASE_H_

#include "def_types.h"
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <memory.h>

typedef struct {
    float x, y;
} HPoint2f;


typedef struct {
    int x;
    int y;
    int width;
    int height;
} HRect;


#define MAX_SHAPE_POINTS 101

typedef struct {
    HPoint2f pts[MAX_SHAPE_POINTS];
    int ptsSize;
} Shape;


typedef struct {
    float scale;
    float sina;
    float cosa;

    HPoint2f cen1;
    HPoint2f cen2;
} TranArgs;


HPoint2f calculate_central(Shape &shape);
void similarity_transform(Shape &srcShape, Shape &dstShape, TranArgs &arg);
HRect get_shape_rect(Shape &shape);

int read_pts_file(const char *filePath, Shape &shape);
int write_pts_file(const char *filePath, Shape &shape);

void affine_shape(Shape &shapeSrc, HPoint2f cen1, Shape &shapeRes, HPoint2f cen2, float scale, float angle);

void calculate_mean_shape_global(Shape *shapes, int size, int ptsSize, int SHAPE_SIZE, float factor, Shape &meanShape);

#endif
