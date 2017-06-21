#include "tool.h"
#include "face_tool.h"

#define WIN_SIZE 96

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
    cv::RNG rng(cv::getTickCount());

    ret = read_file_list(argv[1], imgList);
    if(ret != 0) return 1;

    size = imgList.size();

    for(int i = 0; i < size; i++)
    {
        printf("%6.2f%%\r", 100.0f * i / size);fflush(stdout);

        const char *imgPath = imgList[i].c_str();
        cv::Mat img = cv::imread(imgPath);
        cv::Mat patch;
        Shape shape;

        analysis_file_path(imgPath, rootDir, fileName, ext);

        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        int ptsSize = read_pts_file(filePath, shape);
        if(ptsSize == 0) continue;

        float scale = 1.0f;
        while (scale < 1.4 && scale > 0.6)
            scale = rng.uniform(0.4, 1.6);

        float cx = 0;
        float cy = 0;

        for(int p = 0; p < ptsSize; p++){
            cx += shape.pts[p].x;
            cy += shape.pts[p].y;
        }

        cx /= ptsSize;
        cy /= ptsSize;

        for(int p = 0; p < ptsSize; p++){
            shape.pts[p].x = (shape.pts[p].x - cx) * scale + cx;
            shape.pts[p].y = (shape.pts[p].y - cy) * scale + cy;
        }

        normalize_sample(img, patch, WIN_SIZE, 1.5, shape);

        sprintf(filePath, "%s/%s.jpg", argv[2], fileName);
        cv::imwrite(filePath, patch);
    }

    return 0;
}
