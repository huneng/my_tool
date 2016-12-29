#include "tool.h"
#include <math.h>
#include <stdlib.h>

#include <inttypes.h>

#include <vector>
#include <string>


typedef struct
{
    int width;
    int height;
} HuSize;



HuSize get_image_rotation_box(cv::Mat &img, float angle)
{
    HuSize size;

    int dstx[4], dsty[4];
    int srcx[4], srcy[4];

    float sina = sin(angle);
    float cosa = cos(angle);

    srcx[0] = (int)(img.cols * (-0.5));
    srcy[0] = (int)(img.rows * (-0.5));

    srcx[1] = (int)(img.cols * 0.5);
    srcy[1] = (int)(img.rows * (-0.5));

    srcx[2] = (int)(img.cols * 0.5);
    srcy[2] = (int)(img.rows * 0.5);

    srcx[3] = (int)(img.cols * (-0.5));
    srcy[3] = (int)(img.rows * 0.5);


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


void rotate_image(cv::Mat &src, cv::Mat &dst, float angle)
{
    int srcW, srcH, halfSW, halfSH;
    int dstW, dstH, halfDW, halfDH;

    float sina = sin(angle);
    float cosa = cos(angle);

    int i, j;
    int y, x;
    int y0, x0, x1, y1;
    int dx, dy;

    HuSize size = get_image_rotation_box(src, angle);

    dst = cv::Mat(size.height, size.width, CV_8UC1, cv::Scalar(255));

    srcW = src.cols;
    srcH = src.rows;
    dstW = dst.cols;
    dstH = dst.rows;


    halfSW = srcW / 2;
    halfSH = srcH / 2;
    halfDW = dstW / 2;
    halfDH = dstH / 2;


    for(j = 0; j < dstH; j++)
    {
        y = j - halfDH;

        for(i = 0; i < dstW; i++)
        {
            x = i - halfDW;

            x0 = x * cosa - y * sina + halfSW;
            y0 = x * sina + y * cosa + halfSH;

            x1 = x0 + 1;
            y1 = y0 + 1;

            x1 = HU_MIN(x1, srcW-1);
            y1 = HU_MIN(y1, srcH-1);

            dx = x1 - x0;
            dy = y1 - y0;

            if(0 <= x0 && x1 < srcW-1 && 0 <= y0 && y1 < srcH-1)
            {
                int B1 = src.at<uchar>(y0, x0);
                int B2 = src.at<uchar>(y0, x1);
                int B3 = src.at<uchar>(y1, x0);
                int B4 = src.at<uchar>(y1, x1);

                float t1, t4;

                t1 = B1 * (1.f - dx) + B2 * dx;
                t4 = B3 * (1.f - dx) + B4 * dx;

                dst.at<uchar>(j, i) = (1.f - dy) * t1 + dy * t4 + 0.5f;
            }
        }
    }
}


int main(int argc, char **argv)
{
    if(argc<3)
    {
        printf("Usage:%s [image list] [output dir] \n", argv[0]);
        return 1;
    }

    std::vector<std::string> imageList;

    cv::Mat img, dst;

    char *outdir, outname[128], imgname[40], imgdir[128], ext[20];
    int len, size;

    outdir = argv[2];
    len = strlen(outdir);

    if(outdir[len-1] == '/')
        outdir[len-1] = '\0';


    read_file_list(argv[1], imageList);

    size = imageList.size();


    for(int i = 0; i < size; i++)
    {
        img = cv::imread(imageList[i], 0);

        analysis_file_path(imageList[i].c_str(), imgdir, imgname, ext);

        for(int j = 5; j < 360; j += 5)
        {
            float angle = j * 5 * HU_PI / 180;

            rotate_image(img, dst, angle);

            sprintf(outname, "%s/rotate_%d/%s.bmp", outdir, j, imgname);

            cv::imwrite(outname, dst);
        }

        printf("%d\r", i);
        fflush(stdout);
    }

    return 0;
}



