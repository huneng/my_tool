#include "base.h"


HPoint2f calculate_central(Shape &shape){
    HPoint2f cen;

    cen.x = 0.0f;
    cen.y = 0.0f;

    for(int i = 0; i < shape.ptsSize; i++){
        cen.x += shape.pts[i].x;
        cen.y += shape.pts[i].y;
    }

    cen.x /= shape.ptsSize;
    cen.y /= shape.ptsSize;

    return cen;
}


void similarity_transform(Shape &srcShape, Shape &dstShape, TranArgs &arg){
    float scx, scy;
    float dcx, dcy;

    float var1, var2;
    float sina, cosa;
    int ptsSize;
    float factor;

    assert(srcShape.ptsSize == dstShape.ptsSize);

    ptsSize = srcShape.ptsSize;
    factor = 1.0f / ptsSize;

    scx = 0, scy = 0;
    dcx = 0, dcy = 0;

    for(int i = 0; i < ptsSize; i++){
        scx += srcShape.pts[i].x;
        scy += srcShape.pts[i].y;

        dcx += dstShape.pts[i].x;
        dcy += dstShape.pts[i].y;
    }

    scx *= factor;
    scy *= factor;
    dcx *= factor;
    dcy *= factor;

    var1 = 0, var2 = 0;

    for(int i = 0; i < dstShape.ptsSize; i++){
        float sx = srcShape.pts[i].x - scx;
        float sy = srcShape.pts[i].y - scy;

        float dx = dstShape.pts[i].x - dcx;
        float dy = dstShape.pts[i].y - dcy;

        var1 += sqrtf(sx * sx + sy * sy);
        var2 += sqrtf(dx * dx + dy * dy);

        sina += sy * dx - sx * dy;
        cosa += sx * dx + sy * dy;
    }

    factor = 1.0f / sqrtf(sina * sina +  cosa * cosa);

    arg.scale = var2 / var1;

    arg.sina = sina * factor;
    arg.cosa = cosa * factor;

    arg.cen1.x = scx;
    arg.cen1.y = scy;

    arg.cen2.x = dcx;
    arg.cen2.y = dcy;
}


HRect get_shape_rect(Shape &shape){
    HRect rect;

    float cx = 0, cy = 0, faceSize;

    float minx = FLT_MAX, maxx = -FLT_MAX;
    float miny = FLT_MAX, maxy = -FLT_MAX;

    for(int i = 0; i < shape.ptsSize; i++){
        minx = HU_MIN(minx, shape.pts[i].x);
        maxx = HU_MAX(maxx, shape.pts[i].x);

        miny = HU_MIN(miny, shape.pts[i].y);
        maxy = HU_MAX(maxy, shape.pts[i].y);
    }

    cx = 0.5f * (minx + maxx);
    cy = 0.5f * (miny + maxy);

    faceSize = HU_MAX(maxx - minx + 1, maxy - miny + 1);

    rect.x = cx - 0.5f * faceSize;
    rect.y = cy - 0.5f * faceSize;
    rect.width = faceSize;
    rect.height = faceSize;

    return rect;
}


int read_pts_file(const char *filePath, Shape &shape)
{
    FILE *fin = fopen(filePath, "r");

    char line[256];

    if(fin == NULL){
        printf("Can't open file %s\n", filePath);
        return 0;
    }

    char *ret;
    int ptsSize;
    ret = fgets(line, 255, fin);
    ret = fgets(line, 255, fin);

    sscanf(line, "n_points:  %d", &ptsSize);
    ret = fgets(line, 255, fin);

    shape.ptsSize = ptsSize;

    for(int i = 0; i < ptsSize; i++){
        HPoint2f pt;

        if(fgets(line, 255, fin) == NULL) {
            fclose(fin);
            printf("END of FILE: %s\n", filePath);
            return 0;
        }

        int ret = sscanf(line, "%f %f\n", &pt.x, &pt.y);
        if(ret == 0) break;
        shape.pts[i] = pt;
    }

    fclose(fin);

    return ptsSize;
}


int write_pts_file(const char *filePath, Shape &shape)
{
    FILE *fout = fopen(filePath, "w");

    char line[256];
    int ptsSize;

    if(fout == NULL)
    {
        printf("Can't open file %s\n", filePath);
        return 1;
    }

    ptsSize = shape.ptsSize;

    fprintf(fout, "version: 1\n");
    fprintf(fout, "n_points:  %d\n", ptsSize);
    fprintf(fout, "{\n");

    for(int i = 0; i < ptsSize; i++)
        fprintf(fout, "%f %f\n", shape.pts[i].x, shape.pts[i].y);

    fprintf(fout, "}");
    fclose(fout);

    return 0;
}


void affine_shape(Shape &shapeSrc, HPoint2f cen1, Shape &shapeRes, HPoint2f cen2, float scale, float angle){
    float sina = sin(angle) * scale;
    float cosa = cos(angle) * scale;

    int ptsSize = shapeSrc.ptsSize;

    shapeRes.ptsSize = ptsSize;

    for(int i = 0; i < ptsSize; i++){
        float x = shapeSrc.pts[i].x - cen1.x;
        float y = shapeSrc.pts[i].y - cen1.y;

        shapeRes.pts[i].x =  x * cosa + y * sina + cen2.x;
        shapeRes.pts[i].y = -x * sina + y * cosa + cen2.y;
    }
}


void calculate_mean_shape_global(Shape *shapes, int size, int ptsSize, int SHAPE_SIZE, float factor, Shape &meanShape)
{
    double cxs[MAX_SHAPE_POINTS], cys[MAX_SHAPE_POINTS];

    float minx, miny, maxx, maxy;
    float cx, cy, w, h, faceSize, scale;


    memset(cxs, 0, sizeof(double) * MAX_SHAPE_POINTS);
    memset(cys, 0, sizeof(double) * MAX_SHAPE_POINTS);

    for(int i = 0; i < size; i++){
        Shape &shape = shapes[i];

        minx = FLT_MAX; maxx = -FLT_MAX;
        miny = FLT_MAX; maxy = -FLT_MAX;

        for(int j = 0; j < ptsSize; j++){
            float x = shape.pts[j].x;
            float y = shape.pts[j].y;

            minx = HU_MIN(x, minx);
            maxx = HU_MAX(x, maxx);
            miny = HU_MIN(y, miny);
            maxy = HU_MAX(y, maxy);
        }

        w = maxx - minx + 1;
        h = maxy - miny + 1;

        cx = (maxx + minx) / 2;
        cy = (maxy + miny) / 2;

        faceSize = HU_MAX(w, h);

        scale = SHAPE_SIZE / faceSize;

        for(int j = 0; j < ptsSize; j++){
            cxs[j] += (shape.pts[j].x - cx) * scale + SHAPE_SIZE / 2;
            cys[j] += (shape.pts[j].y - cy) * scale + SHAPE_SIZE / 2;
        }
    }

    minx =  FLT_MAX, miny =  FLT_MAX;
    maxx = -FLT_MAX, maxy = -FLT_MAX;

    meanShape.ptsSize = ptsSize;

    for(int j = 0; j < ptsSize; j++){
        meanShape.pts[j].x = cxs[j] / size;
        meanShape.pts[j].y = cys[j] / size;

        minx = HU_MIN(minx, meanShape.pts[j].x);
        maxx = HU_MAX(maxx, meanShape.pts[j].x);

        miny = HU_MIN(miny, meanShape.pts[j].y);
        maxy = HU_MAX(maxy, meanShape.pts[j].y);
    }

    w = maxx - minx + 1;
    h = maxy - miny + 1;

    cx = (maxx + minx) * 0.5f;
    cy = (maxy + miny) * 0.5f;

    faceSize = HU_MAX(w, h) * factor;

    scale = SHAPE_SIZE / faceSize;

    for(int j = 0; j < ptsSize; j++){
        meanShape.pts[j].x = (meanShape.pts[j].x - cx) * scale + (SHAPE_SIZE >> 1);
        meanShape.pts[j].y = (meanShape.pts[j].y - cy) * scale + (SHAPE_SIZE >> 1);
    }
}

