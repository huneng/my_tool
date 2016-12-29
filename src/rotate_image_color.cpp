#include "tool.h"

#include <math.h>
#include <stdlib.h>

#include <inttypes.h>

#include <vector>
#include <string>

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
            uint8_t* ptrSrcData00 = src->data + y0 * src->stride + x0 * 3;
            uint8_t* ptrSrcData01 = src->data + y0 * src->stride + x0 * 3;
            uint8_t* ptrSrcData10 = src->data + y0 * src->stride + x0 * 3;
            uint8_t* ptrSrcData11 = src->data + y0 * src->stride + x0 * 3;

            for(int i = 0; i < 3; i++)
            {
                float t1, t2, t;

                t1 = (1-wx) * ptrSrcData00[i] + wx * ptrSrcData01[i];
                t2 = (1-wx) * ptrSrcData10[i] + wx * ptrSrcData11[i];

                t = (1-wy) * t1 + wy * t2;

                ptrResData[i] = round(t);
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



int main(int argc, char **argv)
{
    if(argc < 3)
    {
        printf("Usage:%s [image list] [out dir]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imageList;

    char *outdir, outname[128], imgname[40], imgdir[128], ext[20];
    int len, size;

    cv::Mat img, dst;
    HuImage src, res;

    read_file_list(argv[1], imageList);

    size = imageList.size();


    outdir = argv[2];
    len = strlen(outdir);

    if(outdir[len-1] == '/')
        outdir[len-1] = '\0';


    res.width = 0;
    res.height = 0;
    res.data = NULL;

    float delta = 2 * HU_PI / 360;

    for(int i = 0; i < size; i++)
    {
        img = cv::imread(imageList[i]);

        assert(img.channels() == 3);

        mat2huimage(img, &src);

        analysis_file_path(imageList[i].c_str(), imgdir, imgname, ext);

        for(int j = 5; j < 360; j += 5)
        {
            float angle = j * delta;

            rotate_image_color(&src, &res, angle);

            sprintf(outname, "%s/rotate_%d/%s.jpg", outdir, j, imgname);
//            sprintf(outname, "%s/%s_%d.jpg", outdir, imgname, j);

            dst = cv::Mat(res.height, res.width, CV_8UC3, res.data, res.stride);

            cv::imwrite(outname, dst);
        }

        printf("%d\r", i);
        fflush(stdout);
    }

    if(res.data != NULL)
        free(res.data);

    return 0;
}
