#include "tool.h"
#include "face_tool.h"
#include <stdlib.h>


#define WIN_SIZE 96
#define POINT_RADIUS 4

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
        assert(ptsSize > 0);

        normalize_sample(img, patch, WIN_SIZE << 1, shape);

        cv::Rect rect;

        get_shape_rect(shape, rect);

        rect.x -= 0.05f * rect.width;
        rect.y -= 0.05f * rect.height;

        rect.width *= 1.1f;
        rect.height *= 1.1f;

        sprintf(filePath, "%s/%s.jpg", argv[2], fileName);
        cv::imwrite(filePath, patch(rect));
    }

    return 0;
}

