#include "tool.h"
#include "face_tool.h"
#include <stdlib.h>


#define WIN_SIZE 200


int main(int argc, char **argv)
{
    if(argc < 3)
    {
        printf("Usage: %s [image list] [out dir] [tran size]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imgList;
    int size = 0, ret;
    char filePath[128], rootDir[128], fileName[128], ext[30];
    int TRANS_SIZE = 1;

    ret = read_file_list(argv[1], imgList);
    if(ret != 0) return 1;

    if(argc == 4)
        TRANS_SIZE = atoi(argv[3]);

    size = imgList.size();

    std::vector<Shape> shapes;

    printf("read shapes\n");

    int step = size / 10000;
    step = HU_MAX(1, step);
    for(int i = 0; i < size; i += step){
        printf("%6.2f%%\r", 100.0f * i / size);fflush(stdout);

        Shape shape;
        cv::Rect rect;

        const char *imgPath = imgList[i].c_str();

        analysis_file_path(imgPath, rootDir, fileName, ext);
        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        int ptsSize = read_pts_file(filePath, shape);
        get_shape_rect(shape, rect);

        float scale = WIN_SIZE / float(rect.width);

        for(int j = 0; j < ptsSize; j++)
            shape[j].x *= scale, shape[j].y *= scale;

        shapes.push_back(shape);
    }

    Shape meanShape;

    printf("calculate meanshape\n");
    calc_mean_shape_global(shapes, WIN_SIZE, meanShape);

    printf("Read sample\n");
    for(int i = 0; i < size; i++){
        printf("%6.2f%%\r", 100.0f * i / size);fflush(stdout);

        std::vector<cv::Point2f> shape;
        const char *imgPath = imgList[i].c_str();
        cv::Mat img = cv::imread(imgPath);
        cv::Mat src;
        Shape oriShape;
        if(img.empty()){
            printf("Can't open image %s\n", imgPath);
            continue;
        }

        analysis_file_path(imgPath, rootDir, fileName, ext);

        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        int ptsSize = read_pts_file(filePath, oriShape);

        cv::Rect rect;

        normalize_sample(img, src, WIN_SIZE, oriShape);

        if(TRANS_SIZE == 1){
            src.copyTo(img);
            shape = oriShape;

            TranArgs arg;
            similarity_transform(meanShape, shape, arg);

            cv::resize(img, img, cv::Size(arg.scale * img.cols, arg.scale * img.rows));

            float minx = FLT_MAX, maxx = -FLT_MAX;
            float miny = FLT_MAX, maxy = -FLT_MAX;

            for(int j = 0; j < ptsSize; j++){
                shape[j].x *= arg.scale;
                shape[j].y *= arg.scale;

                minx = HU_MIN(minx, shape[j].x);
                miny = HU_MIN(miny, shape[j].y);

                maxx = HU_MAX(maxx, shape[j].x);
                maxy = HU_MAX(maxy, shape[j].y);
            }

            float cx = (minx + maxx) / 2;
            float cy = (miny + maxy) / 2;

            rect.x = cx - WIN_SIZE / 2;
            rect.y = cy - WIN_SIZE / 2;
            rect.width = WIN_SIZE;
            rect.height = WIN_SIZE;

            sprintf(filePath, "%s/%s.jpg", argv[2], fileName);
            cv::imwrite(filePath, img(rect));
        }
        else {
            for(int t = 0; t < TRANS_SIZE; t++){
                src.copyTo(img);
                shape = oriShape;

                transform_sample(img, shape);

                TranArgs arg;
                similarity_transform(meanShape, shape, arg);

                cv::resize(img, img, cv::Size(arg.scale * img.cols, arg.scale * img.rows));

                float minx = FLT_MAX, maxx = -FLT_MAX;
                float miny = FLT_MAX, maxy = -FLT_MAX;

                for(int j = 0; j < ptsSize; j++){
                    shape[j].x *= arg.scale;
                    shape[j].y *= arg.scale;

                    minx = HU_MIN(minx, shape[j].x);
                    miny = HU_MIN(miny, shape[j].y);

                    maxx = HU_MAX(maxx, shape[j].x);
                    maxy = HU_MAX(maxy, shape[j].y);
                }

                float cx = (minx + maxx) / 2;
                float cy = (miny + maxy) / 2;

                rect.x = cx - WIN_SIZE / 2;
                rect.y = cy - WIN_SIZE / 2;
                rect.width = WIN_SIZE;
                rect.height = WIN_SIZE;

                sprintf(filePath, "%s/%s_%d.jpg", argv[2], fileName, t);
                cv::imwrite(filePath, img(rect));
            }
        }
    }


    return 0;
}

