#include "tool.h"
#include "face_tool.h"
#include <stdlib.h>


#define WIN_SIZE 500
#define POINT_RADIUS 1

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        printf("Usage: %s [image list] [out dir]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imgList;
    int size = 0, ret;
    char filePath[128], rootDir[128], fileName[128], ext[30];

    ret = read_file_list(argv[1], imgList);
    if(ret != 0) return 1;

    size = imgList.size();

    for(int i = 0; i < size; i++)
    {
        printf("%6.2f%%\r", 100.0f * i / size);fflush(stdout);

        std::vector<cv::Point2f> shape;
        const char *imgPath = imgList[i].c_str();
        cv::Mat img = cv::imread(imgPath);
        cv::Mat patch;

        analysis_file_path(imgPath, rootDir, fileName, ext);

        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        int ptsSize = read_pts_file(filePath, shape);
        if(ptsSize == 0) continue;

        normalize_sample(img, patch, WIN_SIZE, 1.2, shape);
//*
        for(int p = 0; p < ptsSize; p++){
            cv::circle(patch, shape[p], POINT_RADIUS, cv::Scalar(0, 255, 0), -1);
            char str[4];
            sprintf(str, "%d", p);
            //cv::putText(patch, str, cv::Point(shape[p].x + 5, shape[p].y), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(255, 0, 0));
        }
// */
        sprintf(filePath, "%s/%s.jpg", argv[2], fileName);
        cv::imwrite(filePath, patch);
    }

    return 0;
}

