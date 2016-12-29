#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>

#define RING_SIZE          12
#define CIRCLE_SIZE        40
#define CIRCLE_MAX_PTS_NUM 48
#define INTERVAL_THRESHOLD 0.1

#define HU_PI 3.1415926

void compute_mean(float* data, int size, float *mean)
{
    int i;
    float sum = 0;

    for(i = 0; i < size; i++)
    {
        sum += data[i];
    }

    *mean = 1.0 * sum / size;
}

void compute_mean(uint8_t* data, int size, float *mean)
{
    int i;
    uint32_t sum = 0;

    for(i = 0; i < size; i++)
    {
        sum += data[i];
    }

    *mean = 1.0 * sum / size;
}


void compute_standard(float *data, int size, float mean, float *var)
{
    int i;
    double  sum = 0;

    for(i = 0; i < size; i++)
    {
        sum += (data[i]-mean) * (data[i]-mean);
    }

    sum /= (size-1);

    *var = sum;
}


void compute_standard(uint8_t *data, int size, float mean, float *var)
{
    int i;
    double  sum = 0;

    for(i = 0; i < size; i++)
    {
        sum += (data[i]-mean) * (data[i]-mean);
    }

    sum /= (size-1);

    *var = sqrt(sum);
}


void sample_circle_points(uint8_t* data, int stride, int width, int height,
        int cx, int cy, int radius, uint8_t* sampleVal, int size)
{

    double delta = 2.0 * HU_PI / size;

    sampleVal[0] = data[cy * stride + cx - radius];

    for(int i = 1; i < size; i++)
    {
        double angle = i * delta;

        double x = radius * cosl(i * delta);
        double y = radius * sinl(i * delta);

        int x0 = floor(x);
        int y0 = floor(y);
        int x1 = ceil(x);
        int y1 = ceil(y);

        double wy = y - y0;
        double wx = x - x0;

        double t1 = (1 - wx) * data[(cy + y0) * stride + cx + x0] +
            wx * data[(cy + y0) * stride + cx + x1];

        double t2 = (1 - wx) * data[(cy + y1) * stride + cx + x0] +
            wx * data[(cy + y1) * stride + cx + x1];

        double t = (1 - wy) * t1 + wy * t2;

        sampleVal[i] = round(t);
    }
}


int CIRCLE_SAMPLE_PTS_SIZE[CIRCLE_SIZE] = {
    8, 16, 24, 16, 16, 24, 24, 24, 24, 24,
            24, 24, 24, 24, 24, 24, 24, 32, 32, 32,
            32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
            32, 40, 40, 40, 40, 40, 48, 48, 48, 48 };
//total = 1200


void calc_ring_means(uint8_t* data, int stride, int width, int height, int cx, int cy, float *mean)
{
    uint8_t samplePtsVal[CIRCLE_SIZE * CIRCLE_MAX_PTS_NUM];
    uint8_t *ptrSamplePtsVal;

    int ringPtsSize;
    int cIdx;
    int i, j;

    ptrSamplePtsVal = samplePtsVal ;

    for(i = 0; i < CIRCLE_SIZE; i++)
    {
        sample_circle_points(data, stride, width, height,
                            cx, cy, i + 1,
                            ptrSamplePtsVal, CIRCLE_SAMPLE_PTS_SIZE[i]);

        ptrSamplePtsVal += CIRCLE_SAMPLE_PTS_SIZE[i];
    }

    ptrSamplePtsVal = samplePtsVal;

    for(i = 0; i < CIRCLE_SIZE; i++)
    {
        compute_mean(samplePtsVal, CIRCLE_SAMPLE_PTS_SIZE[i], mean+i);
        printf("%f ", mean[i]);
    }
    printf("\n");
}


void read_image_list_file(const char *fileName, std::vector<std::string> &imageList)
{
    FILE *fp = fopen(fileName, "r");
    char imgpath[256];

    if(fp == NULL)
    {
        printf("Can't open file %s\n", fileName);
        exit(0);
    }

    while(fscanf(fp, "%s\n", imgpath) != EOF)
    {
        imageList.push_back(imgpath);
    }

    fclose(fp);
}


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Usage:%s [image list]\n", argv[1]);
        return 1;
    }

    std::vector<std::string> imageList;

    read_image_list_file(argv[1], imageList);

    int size = imageList.size();

    cv::Mat img;

    float mean[CIRCLE_SIZE];

    float *arr = new float[size];
    uint8_t samplePtsVal[CIRCLE_SIZE];

    for(int i = 0; i < size; i++)
    {
        img = cv::imread(imageList[i], 0);

        if(img.empty())
        {
            printf("Can't read image %s\n", imageList[i].c_str());
            continue;
        }

        calc_ring_means(img.data, img.cols, img.rows, img.step, img.cols/2, img.rows/2, mean);
        arr[i] = mean[1];
    }

    float arrMean, arrVar;

    compute_mean(arr, size, &arrMean);

    compute_standard(arr, size, arrMean, &arrVar);

    printf("%f\n", arrVar);

    return 0;
}
