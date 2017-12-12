#include "tool.h"
#include "face_tool.h"


#define WIN_SIZE 100
#define FACTOR 1.1f

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

    printf("WIN_SIZE = %d, FACTOR = %f\n", WIN_SIZE, FACTOR);
    ret = read_file_list(argv[1], imgList);
    if(ret != 0) return 1;

    size = imgList.size();

    int BUF_SIZE = 20000;
    Shape *shapes = new Shape[BUF_SIZE];

    int step = size / BUF_SIZE, count = 0;

    step = HU_MAX(step, 1);

    for(int i = 0; i < size && count < BUF_SIZE; i += step){
        const char *imgPath = imgList[i].c_str();
        Shape shape;
        analysis_file_path(imgPath, rootDir, fileName, ext);

        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        int ptsSize = read_pts_file(filePath, shape);
        if(ptsSize == 0) continue;

        shapes[count++] = shape;
        printf("%d\r", count);fflush(stdout);
    }

    Shape meanShape;

    calculate_mean_shape_global(shapes, count, shapes[0].ptsSize, WIN_SIZE, FACTOR, meanShape);


    delete [] shapes;

    for(int i = 0; i < size; i++)
    {
        printf("%6.2f%%\r", 100.0f * i / size);fflush(stdout);

        const char *imgPath = imgList[i].c_str();
        cv::Mat img = cv::imread(imgPath);
        cv::Mat patch;
        Shape shape;
        TranArgs arg;

        analysis_file_path(imgPath, rootDir, fileName, ext);

        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        int ptsSize = read_pts_file(filePath, shape);
        if(ptsSize == 0) continue;

        normalize_sample(img, patch, WIN_SIZE << 1, 2.0, shape);

        similarity_transform(shape, meanShape, arg);

        cv::resize(patch, patch, cv::Size(patch.cols * arg.scale, patch.rows * arg.scale));

        float minx = FLT_MAX, maxx = -FLT_MAX;
        float miny = FLT_MAX, maxy = -FLT_MAX;

        float cx, cy;

        for(int p = 0; p < shape.ptsSize; p++){
            shape.pts[p].x *= arg.scale;
            shape.pts[p].y *= arg.scale;

            minx = HU_MIN(minx, shape.pts[p].x);
            maxx = HU_MAX(maxx, shape.pts[p].x);

            miny = HU_MIN(miny, shape.pts[p].y);
            maxy = HU_MAX(maxy, shape.pts[p].y);
        }

        cx = 0.5f * (minx + maxx);
        cy = 0.5f * (miny + maxy);

        cv::Rect rect;

        rect.x = cx - (WIN_SIZE >> 1);
        rect.y = cy - (WIN_SIZE >> 1);
        rect.width = rect.height = WIN_SIZE;

        sprintf(filePath, "%s/%s.jpg", argv[2], fileName);
        cv::imwrite(filePath, patch(rect));
    }

    return 0;
}

