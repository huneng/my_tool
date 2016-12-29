#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define HU_MAX(a, b) ((a) < (b) ? (a) : (b))
#define HU_MIN(a, b) ((a) < (b) ? (a) : (b))


void resize(uint8_t *srcData, int srcWidth, int srcHeight, int srcStride,
        uint8_t *dstData, int dstWidth, int dstHeight, int dstStride);
void resize2(uint8_t *srcData, int srcWidth, int srcHeight, int srcStride,
        uint8_t *dstData, int dstWidth, int dstHeight, int dstStride);


#define RWIDTH 96
#define RHEIGHT 96

int main(int argc, char **argv){
    cv::Mat img(133, 153, CV_8UC1, cv::Scalar(255));
    cv::Mat dst(152, 175, CV_8UC1, cv::Scalar(0));

    resize(img.data, 153, 133, img.step, dst.data, 175, 152, img.step);

    return 0;
}


int main_a(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [input image] [output image]\n", argv[0]);
        return 1;
    }

    cv::Mat img = cv::imread(argv[1], 0);

    cv::Mat out;
    out = cv::Mat(RHEIGHT, RWIDTH, CV_8UC1, cv::Scalar(0));

    time_t start  = clock();
    cv::resize(img, out, cv::Size(out.cols, out.rows));
    time_t end = clock();
    printf("%f \n", (end -start + 0.0f) / CLOCKS_PER_SEC * 1000);
    cv::imwrite("0.jpg", out);

    out = (RHEIGHT, RWIDTH, CV_8UC1, cv::Scalar(0));
    start = clock();
    resize(img.data, img.cols, img.rows, img.step, out.data, out.cols, out.rows, out.step);
    end = clock();
    printf("%f \n", (end -start + 0.0f) / CLOCKS_PER_SEC * 1000);
    cv::imwrite("1.jpg", out);
/*
    out = (RHEIGHT, RWIDTH, CV_8UC1, cv::Scalar(0));
    start = clock();
    resize2(img.data, img.cols, img.rows, img.step, out.data, out.cols, out.rows, out.step);
    end = clock();
    printf("%f \n", (end -start + 0.0f) / CLOCKS_PER_SEC * 1000);
    cv::imwrite("2.jpg", out);
*/
    return 0;
}

void resize2(uint8_t *srcData, int srcWidth, int srcHeight, int srcStride,
        uint8_t *dstData, int dstWidth, int dstHeight, int dstStride)
{
    float scaleX, scaleY;
    float wx0, wx1, wy0, wy1;
    float cx = 0.0f, cy = 0.0f;
    int x0, y0;
    float value1, value2;
    uint8_t *ptrData = NULL, *ptrData2 = NULL;

    assert(srcData != NULL && dstData != NULL);
    assert(srcWidth > 0 && dstWidth > 0 && srcHeight > 0 && dstHeight > 0);

    scaleX = ((float)srcWidth) / dstWidth;
    scaleY = ((float)srcHeight) / dstHeight;

    for(int y = 0; y < dstHeight; y++){
        int len = dstWidth - 4;
        int x;

        y0 = int(cy);
        wy0 = cy - y0;
        wy1 = 1.0f - wy0;

        ptrData2 = srcData + y0 * srcStride;

        cx = 0.0f;
        for(x = 0; x < len; x += 4){
            x0 = int(cx);
            wx0 = cx - x0;
            wx1 = 1 - wx0;

            ptrData = ptrData2 + x0;

            value1 = wx0 * ptrData[1]  + wx1 * ptrData[0];
            value2 = wx0 * ptrData[srcStride + 1] + wx1 * ptrData[srcStride];

            dstData[x] = uint8_t(wy0 * value2 + wy1 * value1);
            cx += scaleX;

            x0 = int(cx);
            wx0 = cx - x0;
            wx1 = 1 - wx0;

            ptrData = ptrData2 + x0;

            value1 = wx0 * ptrData[1]  + wx1 * ptrData[0];
            value2 = wx0 * ptrData[srcStride + 1] + wx1 * ptrData[srcStride];

            dstData[x + 1] = uint8_t(wy0 * value2 + wy1 * value1);
            cx += scaleX;

            x0 = int(cx);
            wx0 = cx - x0;
            wx1 = 1 - wx0;

            ptrData = ptrData2 + x0;

            value1 = wx0 * ptrData[1]  + wx1 * ptrData[0];
            value2 = wx0 * ptrData[srcStride + 1] + wx1 * ptrData[srcStride];

            dstData[x + 2] = uint8_t(wy0 * value2 + wy1 * value1);
            cx += scaleX;

            x0 = int(cx);
            wx0 = cx - x0;
            wx1 = 1 - wx0;

            ptrData = ptrData2 + x0;

            value1 = wx0 * ptrData[1]  + wx1 * ptrData[0];
            value2 = wx0 * ptrData[srcStride + 1] + wx1 * ptrData[srcStride];

            dstData[x + 3] = uint8_t(wy0 * value2 + wy1 * value1);
            cx += scaleX;
        }

        for(; x < dstWidth; x++){
            x0 = int(cx);
            wx0 = cx - x0;
            wx1 = 1 - wx0;

            ptrData = ptrData2 + x0;

            value1 = wx0 * ptrData[1]  + wx1 * ptrData[0];
            value2 = wx0 * ptrData[srcStride + 1] + wx1 * ptrData[srcStride];

            dstData[x + 3] = uint8_t(wy0 * value2 + wy1 * value1);
            cx += scaleX;

        }

        dstData += dstStride;
        cy += scaleY;
    }
}


typedef struct {
    float w0, w1;
    int idx;
} AlphaInfo;


AlphaInfo * create_table(int src, int dst){
    AlphaInfo *table = new AlphaInfo[dst];

    float scale = float(src) / dst;
    float idx = 0.0;

    int len = dst - 4;

    for(int i = 0; i <= len; ){
        idx = i * scale;

        table[i].idx = int(idx);
        table[i].w0 = idx - table[i].idx;
        table[i].w1 = 1 - table[i].w0;
        i++;

        idx = i * scale;

        table[i].idx = int(idx);
        table[i].w0 = idx - table[i].idx;
        table[i].w1 = 1 - table[i].w0;
        i++;

        idx = i * scale;

        table[i].idx = int(idx);
        table[i].w0 = idx - table[i].idx;
        table[i].w1 = 1 - table[i].w0;
        i++;

        idx = i * scale;

        table[i].idx = int(idx);
        table[i].w0 = idx - table[i].idx;
        table[i].w1 = 1 - table[i].w0;
        i++;
    }

    for(int i = len + 1; i < dst; i++){
         idx = i * scale;

        table[i].idx = int(idx);
        table[i].w0 = idx - table[i].idx;
        table[i].w1 = 1 - table[i].w0;
    }

    return table;
}


void resize(uint8_t *srcData, int srcWidth, int srcHeight, int srcStride,
        uint8_t *dstData, int dstWidth, int dstHeight, int dstStride)
{
    float scaleX, scaleY;
    float wx0, wx1, wy0, wy1;
    float value1, value2;
    uint8_t *ptrData = NULL, *ptrData2 = NULL;
    AlphaInfo *colt = create_table(srcWidth, dstWidth);
    AlphaInfo *rowt = create_table(srcHeight, dstHeight);

    assert(srcData != NULL && dstData != NULL);
    assert(srcWidth > 0 && dstWidth > 0 && srcHeight > 0 && dstHeight > 0);

    for(int y = 0; y < dstHeight; y++){
        int idx, idx2;
        int len = dstWidth - 4;
        wy0 = rowt[y].w0;
        wy1 = rowt[y].w1;
        idx = rowt[y].idx * srcStride;

        for(int x = 0; x <= len; ){
            idx2 = idx + colt[x].idx;

            value1 = colt[x].w1 * srcData[idx2] + colt[x].w0 * srcData[idx2 + 1]; idx2 += srcStride;
            value2 = colt[x].w1 * srcData[idx2] + colt[x].w0 * srcData[idx2 + 1];

            dstData[x] = wy1 * value1 + wy0 * value2; x++;

            idx2 = idx + colt[x].idx;

            value1 = colt[x].w1 * srcData[idx2] + colt[x].w0 * srcData[idx2 + 1]; idx2 += srcStride;
            value2 = colt[x].w1 * srcData[idx2] + colt[x].w0 * srcData[idx2 + 1];

            dstData[x] = wy1 * value1 + wy0 * value2; x++;

            idx2 = idx + colt[x].idx;

            value1 = colt[x].w1 * srcData[idx2] + colt[x].w0 * srcData[idx2 + 1]; idx2 += srcStride;
            value2 = colt[x].w1 * srcData[idx2] + colt[x].w0 * srcData[idx2 + 1];

            dstData[x] = wy1 * value1 + wy0 * value2; x++;

            idx2 = idx + colt[x].idx;

            value1 = colt[x].w1 * srcData[idx2] + colt[x].w0 * srcData[idx2 + 1]; idx2 += srcStride;
            value2 = colt[x].w1 * srcData[idx2] + colt[x].w0 * srcData[idx2 + 1];

            dstData[x] = wy1 * value1 + wy0 * value2; x++;
        }

        for(int x = len + 1; x < dstWidth; x++){
            idx2 = idx + colt[x].idx;

            value1 = colt[x].w1 * srcData[idx2] + colt[x].w0 * srcData[idx2 + 1]; idx2 += srcStride;
            value2 = colt[x].w1 * srcData[idx2] + colt[x].w0 * srcData[idx2 + 1];

            dstData[x] = wy1 * value1 + wy0 * value2;
        }

        dstData += dstStride;
    }

    delete [] colt;
    delete [] rowt;
}



