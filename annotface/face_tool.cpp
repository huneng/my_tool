#include "face_tool.h"
#include "tool.h"


int read_pts_file(const char *filePath, std::vector<cv::Point2f> &shapes)
{
    FILE *fin = fopen(filePath, "r");

    char line[256];
    cv::Point2f shape;

    if(fin == NULL){
        printf("Can't open file %s\n", filePath);
        return 0;
    }

    char *ret = fgets(line, 255, fin);

    ret = fgets(line, 255, fin);
    ret = fgets(line, 255, fin);

    shapes.clear();

    while(fgets(line, 255, fin) != NULL)
    {
        int ret = sscanf(line, "%f %f", &shape.x, &shape.y);
        if(ret == 0) break;
        shapes.push_back(shape);
    }

    fclose(fin);

    return shapes.size();
}


int write_pts_file(const char *filePath, std::vector<cv::Point2f> &shapes)
{
    FILE *fout = fopen(filePath, "w");

    char line[256];
    cv::Point2f shape;

    if(fout == NULL)
    {
        printf("Can't open file %s\n", filePath);
        return 1;
    }

    fprintf(fout, "version: 1\n");
    fprintf(fout, "n_points:  %ld\n", shapes.size());
    fprintf(fout, "{\n");


    int size = shapes.size();

    for(int i = 0; i < size; i++)
    {
        fprintf(fout, "%f %f\n", shapes[i].x, shapes[i].y);
    }

    fprintf(fout, "}");
    fclose(fout);

    return 0;
}


//#define USE_CENTRAL

int calc_face_rect(int width, int height, std::vector<cv::Point2f> &shape, cv::Rect &rect){
    int size = shape.size();

    float minx = shape[0].x, miny = shape[0].y;
    float maxx = shape[0].x, maxy = shape[0].y;

    float cx = 0.0f, cy = 0.0f;

    for(int i = 0; i < size; i++){
        minx = HU_MIN(minx, shape[i].x);
        miny = HU_MIN(miny, shape[i].y);

        maxx = HU_MAX(maxx, shape[i].x);
        maxy = HU_MAX(maxy, shape[i].y);

#if defined(USE_CENTRAL)
        cx += shape[i].x;
        cy += shape[i].y;
#endif
    }

#if defined(USE_CENTRAL)
    cx /= size;
    cy /= size;
    int w = HU_MAX(cx - minx, maxx - cx);
    int h = HU_MAX(cy - miny, maxy - cy);

    int faceSize = HU_MAX(w, h) * 1.4;
#else
    cx = (minx + maxx) / 2;
    cy = (miny + maxy) / 2;

    int w = (maxx - minx + 1);
    int h = (maxy - miny + 1);

    int faceSize = HU_MAX(w, h) / 2 * 1.4;
#endif

    rect.x = cx - faceSize;
    rect.y = cy - faceSize;

    rect.height = rect.width = faceSize * 2 + 1;

    if(rect.x < 0 || rect.y < 0 || rect.x + rect.width > width || rect.y + rect.height > height) return 1;

    return 0;
}


void normalize_sample(cv::Mat &src, cv::Mat &patch, std::vector<cv::Point2f> &shape, float factor){
    cv::Mat img;
    src.copyTo(img);

    int width = img.cols;
    int height = img.rows;

    int size = shape.size();

    float minx = shape[0].x, miny = shape[0].y;
    float maxx = shape[0].x, maxy = shape[0].y;

    float cx = 0.0f, cy = 0.0f;

    int bl = 0, br = 0, bt = 0, bb = 0, w, h, faceSize;

    for(int i = 0; i < size; i++){
        minx = HU_MIN(minx, shape[i].x);
        miny = HU_MIN(miny, shape[i].y);

        maxx = HU_MAX(maxx, shape[i].x);
        maxy = HU_MAX(maxy, shape[i].y);
    }

    cx = (minx + maxx) / 2;
    cy = (miny + maxy) / 2;

    w = ceil(maxx - minx + 1);
    h = ceil(maxy - miny + 1);

    faceSize = ceil(HU_MAX(w, h)) * factor;

    int x0 = floor(cx - faceSize);
    int y0 = floor(cy - faceSize);

    faceSize = faceSize * 2 + 1;

    int x1 = x0 + faceSize - 1;
    int y1 = y0 + faceSize - 1;

    patch = cv::Mat(faceSize, faceSize, img.type(), cv::Scalar::all(0));

    if(x0 < 0) {
        bl = -x0;
        x0 = 0;
    }
    if(y0 < 0){
        bt = -y0;
        y0 = 0;
    }
    if(x1 > width - 1){
        br = x1 - width + 1;
        x1 = width - 1;
    }
    if(y1 > height - 1){
        bb = y1 - height + 1;
        y1 = height - 1;
    }

    w = faceSize - bl - br;
    h = faceSize - bt - bb;

    cv::Rect rect1(bl, bt, w, h);
    cv::Rect rect2(x0, y0, w, h);

    patch(rect1) = patch(rect1) + img(rect2);

    float scale = 1;

    scale = 400.0 / faceSize;
    resize(patch, patch, cv::Size(scale * patch.cols, scale * patch.rows));

    for(int i = 0; i < size; i++){
        shape[i].x = (shape[i].x - x0 + bl) * scale;
        shape[i].y = (shape[i].y - y0 + bt) * scale;
    }
}


int write_sample(cv::Mat &img, std::vector<cv::Point2f> &shape, const char *outDir, const char *outName){
    char filePath[256];

#if defined(WIN32)
    sprintf(filePath, "%s\\%s.jpg", outDir, outName);
#elif defined(linux)
    sprintf(filePath, "%s/%s.jpg", outDir, outName);
#endif
    if(!cv::imwrite(filePath, img)){
        printf("Can't write image %s\n", filePath);
        return 1;
    }

#if defined(WIN32)
    sprintf(filePath, "%s\\%s.pts", outDir, outName);
#elif defined(linux)
    sprintf(filePath, "%s/%s.pts", outDir, outName);
#endif
    if(write_pts_file(filePath, shape) != 0){
        printf("Can't write file %s\n", filePath);
        return 2;
    }

    return 0;
}


void mirror_points(std::vector<cv::Point2f> &shape){
    HU_SWAP(shape[0], shape[16], cv::Point2f);
    HU_SWAP(shape[1], shape[15], cv::Point2f);
    HU_SWAP(shape[2], shape[14], cv::Point2f);
    HU_SWAP(shape[3], shape[13], cv::Point2f);
    HU_SWAP(shape[4], shape[12], cv::Point2f);
    HU_SWAP(shape[5], shape[11], cv::Point2f);
    HU_SWAP(shape[6], shape[10], cv::Point2f);
    HU_SWAP(shape[7], shape[9], cv::Point2f);

    HU_SWAP(shape[17], shape[26], cv::Point2f);
    HU_SWAP(shape[18], shape[25], cv::Point2f);
    HU_SWAP(shape[19], shape[24], cv::Point2f);
    HU_SWAP(shape[20], shape[23], cv::Point2f);
    HU_SWAP(shape[21], shape[22], cv::Point2f);

    HU_SWAP(shape[36], shape[45], cv::Point2f);
    HU_SWAP(shape[37], shape[44], cv::Point2f);
    HU_SWAP(shape[38], shape[43], cv::Point2f);
    HU_SWAP(shape[39], shape[42], cv::Point2f);
    HU_SWAP(shape[40], shape[47], cv::Point2f);
    HU_SWAP(shape[41], shape[46], cv::Point2f);

    HU_SWAP(shape[31], shape[35], cv::Point2f);
    HU_SWAP(shape[32], shape[34], cv::Point2f);

    HU_SWAP(shape[48], shape[54], cv::Point2f);
    HU_SWAP(shape[49], shape[53], cv::Point2f);
    HU_SWAP(shape[50], shape[52], cv::Point2f);

    HU_SWAP(shape[59], shape[55], cv::Point2f);
    HU_SWAP(shape[58], shape[56], cv::Point2f);

    HU_SWAP(shape[60], shape[64], cv::Point2f);
    HU_SWAP(shape[61], shape[63], cv::Point2f);

    HU_SWAP(shape[67], shape[65], cv::Point2f);
}


void calc_mean_shape_global(std::vector<Shape > &shapes, int SHAPE_SIZE, Shape &meanShape)
{
    int size = shapes.size();
    int ptsSize = shapes[0].size();

    double cxs[68], cys[68];

    float minx, miny, maxx, maxy;
    float cx, cy, w, h, faceSize, scale;


    assert(ptsSize <= 68);

    meanShape.resize(ptsSize);

    memset(cxs, 0, sizeof(double) * 68);
    memset(cys, 0, sizeof(double) * 68);

    for(int i = 0; i < size; i++){
        Shape &shape = shapes[i];

        minx = FLT_MAX; maxx = -FLT_MAX;
        miny = FLT_MAX; maxy = -FLT_MAX;

        for(int j = 0; j < ptsSize; j++){
            minx = HU_MIN(shape[j].x, minx);
            maxx = HU_MAX(shape[j].x, maxx);
            miny = HU_MIN(shape[j].y, miny);
            maxy = HU_MAX(shape[j].y, maxy);
        }

        w = maxx - minx + 1;
        h = maxy - miny + 1;

        cx = (maxx + minx) / 2;
        cy = (maxy + miny) / 2;

        faceSize = HU_MAX(w, h);

        scale = SHAPE_SIZE / faceSize;

        for(int j = 0; j < ptsSize; j++){
            cxs[j] += (shape[j].x - cx) * scale + SHAPE_SIZE / 2;
            cys[j] += (shape[j].y - cy) * scale + SHAPE_SIZE / 2;
        }
    }

    minx =  FLT_MAX, miny =  FLT_MAX;
    maxx = -FLT_MAX, maxy = -FLT_MAX;

    for(int j = 0; j < ptsSize; j++){
        meanShape[j].x = cxs[j] / size;
        meanShape[j].y = cys[j] / size;

        minx = HU_MIN(minx, meanShape[j].x);
        maxx = HU_MAX(maxx, meanShape[j].x);

        miny = HU_MIN(miny, meanShape[j].y);
        maxy = HU_MAX(maxy, meanShape[j].y);
    }

    w = maxx - minx + 1;
    h = maxy - miny + 1;

    cx = (maxx + minx) / 2;
    cy = (maxy + miny) / 2;

    faceSize = HU_MAX(w, h);

    scale = SHAPE_SIZE / faceSize;

    for(int j = 0; j < ptsSize; j++){
        meanShape[j].x = (meanShape[j].x - cx) * scale + SHAPE_SIZE / 2;
        meanShape[j].y = (meanShape[j].y - cy) * scale + SHAPE_SIZE / 2;
    }
}


void draw_points(cv::Mat &img, std::vector<cv::Point2f> &pts, cv::Scalar color){
    int size = pts.size();

    for(int i = 0; i < size; i++)
        cv::circle(img, pts[i], 2, color);
}


void load_sample(const char *imgPath, cv::Mat &img, Shape &shape){
    char fileName[128], rootDir[128], ext[30], filePath[256];

    img = cv::imread(imgPath, 0);

    analysis_file_path(imgPath, rootDir, fileName, ext);
    sprintf(filePath, "%s/%s.pts", rootDir, fileName);

    read_pts_file(filePath, shape);
}


void similarity_transform(Shape &src, Shape &dst, TranArgs &arg){
    int ptsSize = src.size();

    float cx1 = 0.0f, cy1 = 0.0f;
    float cx2 = 0.0f, cy2 = 0.0f;

    int i;
    for(i = 0; i <= ptsSize - 4; i += 4){
        cx1 += src[i].x;
        cy1 += src[i].y;
        cx1 += src[i + 1].x;
        cy1 += src[i + 1].y;
        cx1 += src[i + 2].x;
        cy1 += src[i + 2].y;
        cx1 += src[i + 3].x;
        cy1 += src[i + 3].y;
    }

    for(; i < ptsSize; i++){
        cx1 += src[i].x;
        cy1 += src[i].y;
    }

    cx1 /= ptsSize;
    cy1 /= ptsSize;

    for(i = 0; i <= ptsSize - 4; i += 4){
        cx2 += dst[i].x;
        cy2 += dst[i].y;
        cx2 += dst[i + 1].x;
        cy2 += dst[i + 1].y;
        cx2 += dst[i + 2].x;
        cy2 += dst[i + 2].y;
        cx2 += dst[i + 3].x;
        cy2 += dst[i + 3].y;
    }

    for(; i < ptsSize; i++){
        cx2 += dst[i].x;
        cy2 += dst[i].y;
    }

    cx2 /= ptsSize;
    cy2 /= ptsSize;

    float ssina = 0.0f, scosa = 0.0f, num = 0.0f;
    float var1 = 0.0f, var2 = 0.0f;

    for(int i = 0; i < ptsSize; i++){
        float sx = src[i].x - cx1;
        float sy = src[i].y - cy1;

        float dx = dst[i].x - cx2;
        float dy = dst[i].y - cy2;

        var1 += sqrtf(sx * sx + sy * sy);
        var2 += sqrtf(dx * dx + dy * dy);

        ssina += (sx * dy - sy * dx);
        scosa += (sx * dx + sy * dy);

        num += sx * sx + dy * dy;
    }

    ssina /= num;
    scosa /= num;

    arg.scale = sqrtf(ssina * ssina + scosa * scosa);

    arg.sina = -ssina / arg.scale;
    arg.cosa =  scosa / arg.scale;

    arg.scale = var1 / var2;

    arg.cen1 = cv::Point2f(cx1, cy1);
    arg.cen2 = cv::Point2f(cx2, cy2);
}


void affine_shape(Shape &shape, float angle, float scale, cv::Point2f &centroid){
    int ptsSize = shape.size();

    float sina = sin(angle) * scale;
    float cosa = cos(angle) * scale;

    for(int i = 0; i < ptsSize; i++){
        float x = shape[i].x - centroid.x;
        float y = shape[i].y - centroid.y;

        shape[i].x =  x * cosa + y * sina + centroid.x;
        shape[i].y = -x * sina + y * cosa + centroid.y;
    }
}


void show_shape(Shape &shape, const char *winName){
    int ptsSize = shape.size();

    float minx = FLT_MAX, maxx = -FLT_MAX;
    float miny = FLT_MAX, maxy = -FLT_MAX;

    for(int i = 0; i < ptsSize; i++){
        minx = HU_MIN(minx, shape[i].x);
        maxx = HU_MAX(maxx, shape[i].x);

        miny = HU_MIN(miny, shape[i].y);
        maxy = HU_MAX(maxy, shape[i].y);
    }

    int faceSize = HU_MAX(maxx - minx + 1, maxy - miny + 1) + 20;


    cv::Mat img(faceSize, faceSize, CV_8UC3, cv::Scalar::all(255));
    for(int i = 0; i < ptsSize; i++)
        cv::circle(img, cv::Point2f(shape[i].x - minx + 10, shape[i].y - miny + 10), 2, cv::Scalar(0, 0, 255), -1);

    cv::imshow(winName, img);
    cv::waitKey();
}
void show_shape(cv::Mat &img, Shape &shape, const char *winName){
    cv::Mat cImg;
    int ptsSize;

    if(img.channels() == 3)
        img.copyTo(cImg);
    else
        cv::cvtColor(img, cImg, cv::COLOR_GRAY2BGR);

    ptsSize = shape.size();

    for(int i = 0; i < ptsSize; i++){
        cv::circle(cImg, shape[i], 3, cv::Scalar(0, 255, 0), -1);
    }

    cv::imshow(winName, cImg);
    cv::waitKey();
}



void show_shape(uint8_t *img, int width, int height, int stride, Shape &shape, const char *winName){
    cv::Mat src, cImg;
    int ptsSize = shape.size();

    src = cv::Mat(height, width, CV_8UC1, img, stride);

    cv::cvtColor(src, cImg, cv::COLOR_GRAY2BGR);

    for(int i = 0; i < ptsSize; i++)
        cv::circle(cImg, shape[i], 3, cv::Scalar(0, 255, 0), -1);

    cv::imshow(winName, cImg);
    cv::waitKey();
}

