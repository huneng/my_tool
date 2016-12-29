#include "tool.h"

#include <stdint.h>
#include <math.h>

#include "face_tool.h"

typedef struct {
    int width;
    int height;
} HuSize;


typedef struct
{
    int width;
    int height;
    int stride;
    uint8_t *data;
} HuImage;


void draw_rect_points(cv::Mat &img, std::vector<cv::Point2f> &shape){

    if(img.channels() == 1){
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    }


    int ptsSize = shape.size();

    float minx, miny, maxx, maxy;
    minx = maxx = shape[0].x;
    miny = miny = shape[0].y;

    for(int i = 0; i < ptsSize; i++){
        cv::circle(img, shape[i], 4, cv::Scalar(0, 3 * i, 0), 2);

        minx = HU_MIN(minx, shape[i].x);
        maxx = HU_MAX(maxx, shape[i].x);
        miny = HU_MIN(miny, shape[i].y);
        maxy = HU_MAX(maxy, shape[i].y);
        cv::imshow("img", img);
        cv::waitKey();
    }

    cv::Rect rect;
    rect.x = int(minx);
    rect.y = int(miny);
    rect.width = int(maxx - minx + 1);
    rect.height = int(maxy - miny + 1);

    cv::rectangle(img, rect, cv::Scalar(255, 0, 0), 2);


}


HuSize get_image_rotation_box(HuImage *img, float angle)
{
    HuSize size;

    int dstx[4], dsty[4];
    int srcx[4], srcy[4];

    float sina = sin(angle);
    float cosa = cos(angle);

    int cx = img->width >> 1;
    int cy = img->height >> 1;

    srcx[0] = -cx;
    srcy[0] = -cy;

    srcx[1] = cx;
    srcy[1] = -cy;

    srcx[2] = cx;
    srcy[2] = cy;

    srcx[3] = -cx;
    srcy[3] = cy;


    dstx[0] = (int)(srcx[0] * cosa - srcy[0] * sina);
    dsty[0] = (int)(srcx[0] * sina + srcy[0] * cosa);

    dstx[1] = (int)(srcx[1] * cosa - srcy[1] * sina);
    dsty[1] = (int)(srcx[1] * sina + srcy[1] * cosa);

    dstx[2] = (int)(srcx[2] * cosa - srcy[2] * sina);
    dsty[2] = (int)(srcx[2] * sina + srcy[2] * cosa);

    dstx[3] = (int)(srcx[3] * cosa - srcy[3] * sina);
    dsty[3] = (int)(srcx[3] * sina + srcy[3] * cosa);


    size.width  = HU_MAX(abs(dstx[2]-dstx[0]), abs(dstx[3]-dstx[1]));
    size.height = HU_MAX(abs(dsty[2]-dsty[0]), abs(dsty[3]-dsty[1]));


    return size;
}


void rotate_image_color(HuImage *src, HuImage *res, float angle)
{
    int srcW, srcH, halfSW, halfSH;
    int dstW, dstH, halfDW, halfDH;

    float sina = sin(angle);
    float cosa = cos(angle);

    int i, j;

    HuSize size = get_image_rotation_box(src, angle);

    if(res->width * res->height < size.width * size.height)
    {
        if(res->data!=NULL)
            free(res->data);

        res->data = (uint8_t*)malloc(sizeof(uint8_t) * size.width * size.height * 3);
    }

    memset(res->data, 0, sizeof(uint8_t) * size.width * size.height * 3);
    assert(res->data != NULL);

    res->width = size.width;
    res->height = size.height;
    res->stride = 3 * size.width;

    srcW = src->width;
    srcH = src->height;
    dstW = res->width;
    dstH = res->height;

    halfSW = srcW / 2;
    halfSH = srcH / 2;
    halfDW = dstW / 2;
    halfDH = dstH / 2;

    for(j = 0; j < dstH; j++)
    {
        float y = j - halfDH;

        for(i = 0; i < dstW; i++)
        {
            float x = i - halfDW;

            float cx = x * cosa - y * sina + halfSW;
            float cy = x * sina + y * cosa + halfSH;

            int x0 = floor(cx);
            int y0 = floor(cy);
            int x1 = ceil(cx);
            int y1 = ceil(cy);


            if(x0 < 0 || x1 >= srcW || y0 < 0 || y1 >= srcH)
                continue;

            float wx = cx - x0;
            float wy = cy - y0;

            uint8_t* ptrResData = res->data + j * res->stride + i * 3;
            uint8_t* ptrSrcData00 = (src->data + y0 * src->stride + x0 * 3);
            uint8_t* ptrSrcData01 = (src->data + y0 * src->stride + x1 * 3);
            uint8_t* ptrSrcData10 = (src->data + y1 * src->stride + x0 * 3);
            uint8_t* ptrSrcData11 = (src->data + y1 * src->stride + x1 * 3);

            assert(0 <= i && i < dstW);
            assert(0 <= j && j < dstH);
            assert(0 <= x0 && x1 < srcW);
            assert(0 <= y0 && y1 < srcH);

            for(int k = 0; k < 3; k++)
            {
                float t1, t2, t;

                t1 = (1-wx) * ptrSrcData00[k] + wx * ptrSrcData01[k];
                t2 = (1-wx) * ptrSrcData10[k] + wx * ptrSrcData11[k];

                t = (1-wy) * t1 + wy * t2;

                ptrResData[k] = ceil(t);
            }
        }
    }
}


void mat2huimage(cv::Mat &src, HuImage *res)
{
    res->width = src.cols;
    res->height = src.rows;
    res->stride = src.step;

    res->data = src.data;
}


void resize_sample(cv::Mat &img, std::vector<cv::Point2f> &shape, int flag){
    int ptsSize = shape.size();

    float factor = 2.0;

    if(flag >= 0 && flag < 4)
        factor = 2.0;
    if(flag >= 4 && flag < 8)
        factor = 1.0;
    if(flag >= 8 && flag < 9)
        factor = 0.5;
    if(flag >= 9 && flag < 10)
        factor = 0.25;

    int winSize = 196 * factor;

    assert(img.cols == img.rows);

    if(img.cols < winSize) return;

    float scalex = float(img.cols) / winSize;
    float scaley = float(img.rows) / winSize;

    for(int i = 0; i < ptsSize; i++){
        shape[i].x /= scalex;
        shape[i].y /= scaley;
    }

    cv::resize(img, img, cv::Size(winSize, winSize));
}





int main_mirror_and_blur(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [image list] [out dir]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imageList;
    int size = 0, ret;

    ret = read_file_list(argv[1], imageList);
    if(ret != 0) return 1;

    size = imageList.size();

    for(int i = 0; i < size; i++){
        char outname[128];
        std::vector<cv::Point2f> shape;

        std::string imgPath = imageList[i];
        std::string imgName = imgPath.substr(0, imgPath.rfind("."));
        std::string ptsPath = imgName + ".pts";
        int ptsSize = 0;
#if defined(WIN32)
        imgName = imgName.substr(imgPath.rfind("\\") + 1);
#elif defined(linux)
        imgName = imgName.substr(imgPath.rfind("/") + 1);
#endif

        cv::Mat img = cv::imread(imgPath, 1);

        if(read_pts_file(ptsPath.c_str(), shape) == 0)
            exit(2);

        cv::Mat patch;
        normalize_sample(img, patch, shape);

        if(patch.channels() == 1)
            cv::cvtColor(patch, patch, cv::COLOR_GRAY2BGR);

        ptsSize = shape.size();

        ret = write_sample(patch, shape, argv[2], imgName.c_str());

        //noise image
        cv::Mat noiseImg;
        patch.copyTo(noiseImg);

        noise_image(noiseImg.data, noiseImg.cols, noiseImg.rows, noiseImg.step);

        sprintf(outname, "%s_noise", imgName.c_str());
        ret = write_sample(noiseImg, shape, argv[2], outname);


        cv::Mat normImg;
        patch.copyTo(normImg);

        npd_normalize(normImg.data, normImg.cols, normImg.rows, normImg.step);

        sprintf(outname, "%s_norm", imgName.c_str());
        ret = write_sample(normImg, shape, argv[2], outname);

        mirror_image(patch.data, patch.cols, patch.rows, patch.step);

        for(int j = 0; j < ptsSize; j++){
            shape[j].x = patch.cols - 1 - shape[j].x;
        }

        mirror_points(shape);

        sprintf(outname, "%s_v", imgName.c_str());
        ret = write_sample(patch, shape, argv[2], outname);


        printf("%.2f%%\r", 100.0 * (i+1)/size);fflush(stdout);
    }

    return 0;
}


int main_random(int argc, char **argv)
{
    if(argc < 4){
        printf("Usage: %s [image list] [size] [out dir]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imageList;
    int size = 0, ret, tranSize;
    cv::RNG rng(cv::getTickCount());

    ret = read_file_list(argv[1], imageList);
    if(ret != 0) return 1;

    tranSize = atoi(argv[2]);

    size = imageList.size();

    HuImage res;
    res.width = res.height = res.stride = 0;
    res.data = NULL;

    for(int i = 0; i < size; i++)
    {
        char outname[128];
        std::vector<cv::Point2f> shape;

        std::string imgPath = imageList[i];
        std::string imgName = imgPath.substr(0, imgPath.rfind("."));
        std::string ptsPath = imgName + ".pts";
#if defined(WIN32)
        imgName = imgName.substr(imgPath.rfind("\\")+1);
#elif defined(linux)
        imgName = imgName.substr(imgPath.rfind("/")+1);
#endif

        cv::Mat img = cv::imread(imgPath, 1);
        assert(img.channels() == 3);

        if( read_pts_file(ptsPath.c_str(), shape) == 0 )
            exit(2);


        cv::Mat patch;
        normalize_sample(img, patch, shape);

        cv::Mat rImg;
        HuImage src;
        int ptsSize = shape.size();

        mat2huimage(patch, &src);

        for(int j = 0; j < tranSize; j++){
            float angle = rng.uniform(0.0, HU_PI/10);
            int blurFlag = rng.uniform(0, 4);
            int noiseFlag = rng.uniform(0, 4);
            //int npdFlag = rng.uniform(0, 8);
            int mirrorFlag = rng.uniform(0, 2);
            int scaleFlag = rng.uniform(0, 11);


            std::vector<cv::Point2f> resShape = shape;

#define ROTATE_IMAGE
#ifdef ROTATE_IMAGE
            rotate_image_color(&src, &res, angle);
            rImg = cv::Mat(res.height, res.width, CV_8UC3, res.data, res.stride);
            rImg.copyTo(rImg);
#else
            patch.copyTo(rImg);

#endif

            //if(npdFlag == 0)
            //    npd_normalize(rImg.data, rImg.cols, rImg.rows, rImg.step);

            if(blurFlag == 0){
                int bsize = rng.uniform(1, 4);
                bsize = 2 * bsize + 1;
                cv::GaussianBlur(rImg, rImg, cv::Size(bsize, bsize), 0, 0);
            }

            if(noiseFlag == 0)
                noise_image(rImg.data, rImg.cols, rImg.rows, rImg.step);

            //mirror
            if(mirrorFlag == 1)
                mirror_image(rImg.data, rImg.cols, rImg.rows, rImg.step);

#ifdef ROTATE_IMAGE
            float cosa = cos(angle);
            float sina = sin(angle);

            for(int k = 0; k < ptsSize; k++){
                float x = resShape[k].x - patch.cols / 2;
                float y = resShape[k].y - patch.rows / 2;

                resShape[k].x = (cosa * x + sina * y) + rImg.cols / 2;
                resShape[k].y = (-sina * x + cosa * y) + rImg.rows / 2;
            }
#endif
            if(mirrorFlag == 1){
                for(int k = 0; k < ptsSize; k++)
                    resShape[k].x = rImg.cols - 1 - resShape[k].x;

                mirror_points(resShape);
            }


            resize_sample(rImg, resShape, scaleFlag);

            //            printf("angle = %f, scale = %f, blur = %d, mirror = %d\n", angle, scale, blurSize, mirrorFlag);

            sprintf(outname, "%s_%d", imgName.c_str(), j);

            ret = write_sample(rImg, resShape, argv[3], outname);

            if(ret != 0) break;
        }

        printf("%d\r", i); fflush(stdout);
    }

    return 0;
}


int main(int argc, char **argv){
    //main_mirror_and_blur(argc, argv);
    main_random(argc, argv);

    return 0;
}
