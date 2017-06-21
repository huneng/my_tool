#include "face_tool.h"
#include "tool.h"


void normalize_sample(cv::Mat &src, cv::Mat &patch, int winSize, float factor, Shape &shape){
    HRect rect;
    int width  = src.cols;
    int height = src.rows;
    int faceSize;

    int bl, bt, br, bb;
    int x0, y0, x1, y1;
    int w, h;

    float scale = 1;
    int ptsSize;

    rect = get_shape_rect(shape);

    faceSize = rect.width * factor;

    int cx = rect.x + (rect.width  >> 1);
    int cy = rect.y + (rect.height >> 1);

    x0 = cx - (faceSize >> 1);
    y0 = cy - (faceSize >> 1);
    x1 = x0 + faceSize - 1;
    y1 = y0 + faceSize - 1;

    bl = 0, bt = 0, br = 0, bb = 0;

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

    patch = cv::Mat(faceSize, faceSize, src.type(), cv::Scalar::all(0));

    patch(rect1) = patch(rect1) + src(rect2);

    scale = float(winSize) / faceSize;
    resize(patch, patch, cv::Size(scale * patch.cols, scale * patch.rows));

    ptsSize = shape.ptsSize;

    for(int i = 0; i < ptsSize; i++){
        shape.pts[i].x = (shape.pts[i].x - x0 + bl) * scale;
        shape.pts[i].y = (shape.pts[i].y - y0 + bt) * scale;
    }
}


int save_sample(cv::Mat &img, Shape &shape, const char *outDir, const char *outName){
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


void load_sample(const char *imgPath, cv::Mat &img, Shape &shape){
    char fileName[128], rootDir[128], ext[30], filePath[256];

    img = cv::imread(imgPath, 0);

    analysis_file_path(imgPath, rootDir, fileName, ext);
    sprintf(filePath, "%s/%s.pts", rootDir, fileName);

    read_pts_file(filePath, shape);
}


void show_shape(Shape &shape, const char *winName){
    int ptsSize = shape.ptsSize;

    HRect rect = get_shape_rect(shape);

    int faceSize = rect.width + 20;

    faceSize = HU_MAX(96, faceSize);
    printf("%d\n", faceSize);

    cv::Mat img(faceSize, faceSize, CV_8UC3, cv::Scalar::all(255));
    for(int i = 0; i < ptsSize; i++)
        cv::circle(img, cv::Point2f(shape.pts[i].x - rect.x + 10, shape.pts[i].y - rect.y + 10), 2, cv::Scalar(0, 0, 255), -1);

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

    ptsSize = shape.ptsSize;

    for(int i = 0; i < ptsSize; i++){
        cv::circle(cImg, cv::Point2f(shape.pts[i].x, shape.pts[i].y), 3, cv::Scalar(0, 255, 0), -1);
    }

    cv::imshow(winName, cImg);
    cv::waitKey();
}



void show_shape(uint8_t *img, int width, int height, int stride, Shape &shape, const char *winName){
    cv::Mat src, cImg;
    int ptsSize = shape.ptsSize;

    src = cv::Mat(height, width, CV_8UC1, img, stride);

    cv::cvtColor(src, cImg, cv::COLOR_GRAY2BGR);

    for(int i = 0; i < ptsSize; i++)
        cv::circle(cImg, cv::Point2f(shape.pts[i].x, shape.pts[i].y), 3, cv::Scalar(0, 255, 0), -1);

    cv::imshow(winName, cImg);
    cv::waitKey();
}


int read_samples(const char *listFile, std::vector<Sample*> &samples, int winSize){

    std::vector<std::string> imgList;
    int size;
    char rootDir[128], fileName[128], ext[30], filePath[256];

    read_file_list(listFile, imgList);

    if(imgList.size() == 0){
        printf("Can't open image list file %s\n", listFile);
        return 1;
    }


    size = imgList.size();

    samples.resize(size);

    for(int i = 0; i < size; i++){
        const char *imgPath = imgList[i].c_str();

        analysis_file_path(imgPath, rootDir, fileName, ext);

        cv::Mat img = cv::imread(imgPath, 1);
        if(img.empty()){
            printf("Can't open image %s\n", imgPath);
            continue;
        }

        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        Shape shape;

        if( read_pts_file(filePath, shape) == 0){
            printf("Can't open pts file %s\n", filePath);
            continue;
        }

        samples[i] = new Sample;

        samples[i]->shape = shape;

        normalize_sample(img, samples[i]->img, winSize, 1.5f, samples[i]->shape);

        samples[i]->imgName = std::string(fileName);
    }

    return 0;
}


#define FIX_INTER_POINT 14

void affine_sample(uint8_t *img, int width, int height, int stride, Shape &shape, float scale, float angle, uint8_t *dst){
    int FIX_ONE = 1 << FIX_INTER_POINT;
    int FIX_0_5 = FIX_ONE >> 1;

    float sina = sinf(-angle) / scale;
    float cosa = cosf(-angle) / scale;

    int id = 0;

    int dstw = width;
    int dsth = height;
    int dsts = stride;

    int *xtable = new int[(dstw << 1) + (dsth << 1)]; assert(xtable != NULL);
    int *ytable = xtable + (dstw << 1);

    HPoint2f center;

    center.x = width >> 1;
    center.y = height >> 1;

    int fcx = center.x * FIX_ONE;
    int fcy = center.y * FIX_ONE;

    for(int i = 0; i < dsth; i++){
        int idx = i << 1;

        float y = (i - center.y);

        ytable[idx]     = y * sina * FIX_ONE + fcx;
        ytable[idx + 1] = y * cosa * FIX_ONE + fcy;
    }

    for(int i = 0; i < dstw; i++){
        int idx = i << 1;

        float x = (i - center.x);

        xtable[idx]     = x * sina * FIX_ONE;
        xtable[idx + 1] = x * cosa * FIX_ONE;
    }

    memset(dst, 0, sizeof(uint8_t) * width * height);

    id = 0;
    for(int y = 0; y < dsth; y++){
        int idx = y << 1;

        int ys = ytable[idx]    ;
        int yc = ytable[idx + 1];

        for(int x = 0; x < dstw; x++){
            idx = x << 1;

            int xs = xtable[idx];
            int xc = xtable[idx + 1];

            int fx =  xc + ys;
            int fy = -xs + yc;

            int x0 = fx >> FIX_INTER_POINT;
            int y0 = fy >> FIX_INTER_POINT;

            int wx = fx - (x0 << FIX_INTER_POINT);
            int wy = fy - (y0 << FIX_INTER_POINT);

            if(x0 < 0 || x0 >= width || y0 < 0 || y0 >= height)
                continue;

            assert(wx <= FIX_ONE && wy <= FIX_ONE);

            uint8_t *ptr1 = img + y0 * stride + x0;
            uint8_t *ptr2 = ptr1 + stride;

            uint8_t value0 = ((ptr1[0] << FIX_INTER_POINT) + (ptr1[1] - ptr1[0]) * wx + FIX_0_5) >> FIX_INTER_POINT;
            uint8_t value1 = ((ptr2[0] << FIX_INTER_POINT) + (ptr2[1] - ptr2[0]) * wx + FIX_0_5) >> FIX_INTER_POINT;

            dst[id + x] = ((value0 << FIX_INTER_POINT) + (value1 - value0) * wy + FIX_0_5) >> FIX_INTER_POINT;
        }

        id += dsts;
    }

    delete [] xtable;

    affine_shape(shape, center, shape, center, scale, angle);
}
