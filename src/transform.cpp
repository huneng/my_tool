#include "tool.h"

#include <stdint.h>
#include <omp.h>
#include <math.h>

#include "face_tool.h"

#define WIN_SIZE 288
#define FACTOR 3.0f


static void vertical_mirror(Shape &shape, int width){
    int ptsSize = shape.size();

    for(int i = 0; i < ptsSize; i++)
        shape[i].x = width - 1 - shape[i].x;

    if(ptsSize == 68){
        int idxs1[29] = {  0,  1,  2,  3,  4,  5,  6,  7,
                        17, 18, 19, 20, 21,
                        31, 32,
                        36, 37, 38, 39, 40, 41,
                        48, 49, 50, 58, 59, 60, 61, 67 };

        int idxs2[29] = { 16, 15, 14, 13, 12, 11, 10,  9,
                        26, 25, 24, 23, 22,
                        35, 34,
                        45, 44, 43, 42, 47, 46,
                        54, 53, 52, 56, 55, 64, 63, 65 };


        for(int i = 0; i < 29; i++){
            int id1 = idxs1[i];
            int id2 = idxs2[i];

            HU_SWAP(shape[id1], shape[id2], cv::Point2f);
        }

    }
    else if(ptsSize == 51){
        int idxs1[21] = {
            0, 1, 2, 3, 4,
            14, 15,
            19, 20, 21, 22, 23, 24,
            31, 32, 33, 41, 42, 43, 44, 50};

        int idxs2[21] = {
                        9, 8, 7, 6, 5,
                        18, 17,
                        28, 27, 26, 25, 30, 29,
                        37, 36, 35, 39, 38, 47, 46, 48};

        for(int i = 0; i < 21; i++){
            int id1 = idxs1[i];
            int id2 = idxs2[i];

            HU_SWAP(shape[id1], shape[id2], cv::Point2f);
        }
    }
    else if(ptsSize == 101){
        int idxs1[46] = {
            0, 1, 2, 3, 4, 5, 6, 7, 8,
            19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
            39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 95,
            63, 64, 65, 66, 67, 68,
            75, 76, 77, 85, 86, 87, 88, 94,
        };
        int idxs2[46] = {
            18, 17, 16, 15, 14, 13, 12, 11, 10,
            34, 33, 32, 31, 30, 29, 38, 37, 36, 35,
            57, 56, 55, 54, 53, 52, 51, 62, 61, 60, 59, 58, 96,
            74, 73, 72, 71, 70, 69,
            81, 80, 79, 83, 82, 91, 90, 92
        };

        for(int i = 0; i < 46; i++){
            int id1 = idxs1[i];
            int id2 = idxs2[i];

            HU_SWAP(shape[id1], shape[id2], cv::Point2f);
        }
    }
    else{
        printf("NO PTS FORMAT: %d\n", ptsSize);
        exit(-1);
    }
}


void mirror_sample(uint8_t *img, int width, int height, int stride, Shape &shape){
    mirror_image(img, width, height, stride);
    vertical_mirror(shape, width);
}


void transform_image(uint8_t *img, int width, int height, int stride, Shape &shape){
    static cv::RNG rng(cv::getTickCount());

    assert(stride * height < 4096 * 4096);

    if(rng.uniform(0, 10) == 0){
        uint8_t *data = img;

        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                if(data[x] > 16 && data[x] < 239)
                    data[x] += rng.uniform(-8, 8);
            }

            data += stride;
        }
    }

    if(rng.uniform(0, 10) == 1){
        cv::Mat sImg(height, width, CV_8UC1, img);
        cv::Mat blur;

        int bsize = 2 * rng.uniform(1, 4) + 1;
        cv::GaussianBlur(sImg, blur, cv::Size(bsize, bsize), 0, 0);

        for(int y = 0; y < height; y++)
            memcpy(img + y * stride, blur.data + y * blur.step, sizeof(uint8_t) * width);
    }

    if(rng.uniform(0, 10) == 2){
        mirror_sample(img, width, height, stride, shape);
    }
}

#define FIX_INTER_POINT 14

void affine_sample_color(uint8_t *img, int width, int height, int stride, Shape &shape, float scale, float angle, uint8_t *dst){
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

    cv::Point2f center(width >> 1, height >> 1);

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

            uint8_t *ptr1, *ptr2;
            uint8_t value0, value1;

            ptr1 = img + y0 * stride + x0 * 3;
            ptr2 = ptr1 + stride;

            value0 = ((ptr1[0] << FIX_INTER_POINT) + (ptr1[3] - ptr1[0]) * wx + FIX_0_5) >> FIX_INTER_POINT;
            value1 = ((ptr2[0] << FIX_INTER_POINT) + (ptr2[3] - ptr2[0]) * wx + FIX_0_5) >> FIX_INTER_POINT;

            dst[id + x * 3] = ((value0 << FIX_INTER_POINT) + (value1 - value0) * wy + FIX_0_5) >> FIX_INTER_POINT;

            ptr1++;
            ptr2++;

            value0 = ((ptr1[0] << FIX_INTER_POINT) + (ptr1[3] - ptr1[0]) * wx + FIX_0_5) >> FIX_INTER_POINT;
            value1 = ((ptr2[0] << FIX_INTER_POINT) + (ptr2[3] - ptr2[0]) * wx + FIX_0_5) >> FIX_INTER_POINT;

            dst[id + x * 3 + 1] = ((value0 << FIX_INTER_POINT) + (value1 - value0) * wy + FIX_0_5) >> FIX_INTER_POINT;

            ptr1++;
            ptr2++;

            value0 = ((ptr1[0] << FIX_INTER_POINT) + (ptr1[3] - ptr1[0]) * wx + FIX_0_5) >> FIX_INTER_POINT;
            value1 = ((ptr2[0] << FIX_INTER_POINT) + (ptr2[3] - ptr2[0]) * wx + FIX_0_5) >> FIX_INTER_POINT;

            dst[id + x * 3 + 2] = ((value0 << FIX_INTER_POINT) + (value1 - value0) * wy + FIX_0_5) >> FIX_INTER_POINT;
        }

        id += dsts;
    }

    delete [] xtable;

    affine_shape(shape, center, shape, center, scale, angle);
}

void transform_sample(cv::Mat &img, Shape &shape){
    float angle;
    float scale;

    cv::RNG rng(cv::getTickCount());

    angle = rng.uniform(-HU_PI / 9, HU_PI / 9);
    scale = 1.0f;

    uint8_t *res = new uint8_t[img.step * img.rows];
    memset(res, 0, sizeof(uint8_t) * img.step * img.rows);
    affine_sample_color(img.data, img.cols, img.rows, img.step, shape, scale, angle, res);

    memcpy(img.data, res, sizeof(uint8_t) * img.rows * img.step);

    delete [] res;

    //transform_image(img.data, img.cols, img.rows, img.step, shape);
}


int main(int argc, char **argv){
    if(argc < 4){
        printf("Usage: %s [image list] [size] [out dir]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imgList;
    int size, tsize;

    read_file_list(argv[1], imgList);
    size = imgList.size();

    tsize = atoi(argv[2]);

    int finished = 0;
#pragma omp parallel for num_threads(omp_get_num_procs() - 1)
    for(int i = 0; i < size; i++){
        char rootDir[128], fileName[128], ext[30], filePath[256];
        const char *imgPath = imgList[i].c_str();

        analysis_file_path(imgPath, rootDir, fileName, ext);
        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        cv::Mat img = cv::imread(imgPath, 1);
        if(img.empty()) {
            printf("Can't open image %s\n", imgPath);
            continue;
        }

        Shape shape;
        int ret = read_pts_file(filePath, shape);
        if(ret == 0){
            printf("Can't open file %s\n", filePath);
            continue;
        }

        cv::Mat patch;
        cv::Mat res;
        Shape shapeRes;

        normalize_sample(img, patch, WIN_SIZE, FACTOR, shape);

        for(int t = 0; t < tsize; t++){
            patch.copyTo(res);
            shapeRes = shape;

            transform_sample(res, shapeRes);

            sprintf(filePath, "%s/%s_%02d.jpg", argv[3], fileName, t);
            cv::imwrite(filePath, res);

            sprintf(filePath, "%s/%s_%02d.pts", argv[3], fileName, t);
            write_pts_file(filePath, shapeRes);
        }

#pragma omp critical
        {
            finished++;
            printf("%d\r", finished), fflush(stdout);
        }
    }

    return 0;
}
