#include "tool.h"


void get_image_rotation_box(uint8_t *img, int width, int height, int stride, float angle, int &cols, int &rows){
    int dstx[4], dsty[4];
    int srcx[4], srcy[4];

    float sina = sin(angle);
    float cosa = cos(angle);

    int cx = width >> 1;
    int cy = height >> 1;

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


    cols = HU_MAX(abs(dstx[2]-dstx[0]), abs(dstx[3]-dstx[1]));
    rows = HU_MAX(abs(dsty[2]-dsty[0]), abs(dsty[3]-dsty[1]));
}


void rotate_image_color(uint8_t *src, int srcw, int srch, int srcs, float angle,
        uint8_t *dst, int &dstw, int &dsth, int &dsts)
{
    float sina = sinf(angle);
    float cosa = cosf(angle);
    int hsw, hsh, hdw, hdh;

    get_image_rotation_box(src, srcw, srch, srcs, angle, dstw, dsth);

    dsts = dstw * 3;

    assert((dstw * dsth) < (srcw * srch * 4));

    hsw = srcw >> 1;
    hsh = srch >> 1;
    hdw = dstw >> 1;
    hdh = dsth >> 1;

    for(int j = 0; j < dsth; j++){
        float y = j - hdh;

        for(int i = 0; i < dstw; i++){
            float x = i - hdw;

            float cx = x * cosa - y * sina + hsw;
            float cy = x * sina + y * cosa + hsh;

            int x0 = int(cx);
            int y0 = int(cy);
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            if(x0 < 0 || x1 >= srcw || y0 < 0 || y1 >= srch)
                continue;

            float wx, wy;
            uint8_t *ptrData1, *ptrDst, *ptrData2;
            float value1, value2;

            wx = x1 - cx;
            wy = y1 - cy;

            ptrData1 = src + y0 * srcs + x0 * 3;
            ptrData2 = src + y1 * srcs + x0 * 3;
            ptrDst = dst + j * dsts + i * 3;

            value1 = wx * (ptrData1[0] - ptrData1[3]) + ptrData1[3];
            value2 = wx * (ptrData2[0] - ptrData2[3]) + ptrData2[3];

            ptrDst[0] = uint8_t(wy * (value1 - value2) + value2 + 0.5f);

            value1 = wx * (ptrData1[1] - ptrData1[4]) + ptrData1[4];
            value2 = wx * (ptrData2[1] - ptrData2[4]) + ptrData2[4];

            ptrDst[1] = uint8_t(wy * (value1 - value2) + value2 + 0.5f);

            value1 = wx * (ptrData1[2] - ptrData1[5]) + ptrData1[5];
            value2 = wx * (ptrData2[2] - ptrData2[5]) + ptrData2[5];

            ptrDst[2] = uint8_t(wy * (value1 - value2) + value2 + 0.5f);
        }
    }
}


#include <omp.h>

int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [image list] [out dir]\n", argv[0]);
        return 0;
    }

    std::vector<std::string> imgList;
    int size;
    float delta;

    char rootDir[256], fileName[256], ext[30], filePath[256];

    read_file_list(argv[1], imgList);

    delta = 2.0f * HU_PI / 360.0f;

    size = imgList.size();

//#pragma omp parallel for num_threads(omp_get_num_procs() - 1)
    for(int i = 0; i < size; i++){
        cv::Mat img = cv::imread(imgList[i], 1);

        if(img.empty()){
            printf("Can't open image %s\n", imgList[i].c_str());
            continue;
        }

        analysis_file_path(imgList[i].c_str(), rootDir, fileName, ext);
        uint8_t *dstData;
        int dstw, dsth, dsts;
        int capacity;

        capacity = img.cols * img.rows * 3 * 4;

        dstData = new uint8_t[capacity];

        for(int j = 5; j < 360; j += 5){
            float angle = j * delta;

            dstw = 0, dsts = 0, dsth = 0;

            memset(dstData, 0, sizeof(uint8_t) * capacity);
            rotate_image_color(img.data, img.cols, img.rows, img.step, angle,
                    dstData, dstw, dsth, dsts);

            sprintf(filePath, "%s/%s_%d.jpg", argv[2], fileName, j);

            if(img.cols > 0 && img.rows > 0){
                cv::Mat res(dsth, dstw, CV_8UC3, dstData, dsts);
                if(!cv::imwrite(filePath, res)){
                    printf("Can't write image %s\n", filePath);
                }
            }
        }

        printf("%d\r", i), fflush(stdout);
        delete [] dstData;
    }

    return 0;
}
