
#include <stdio.h>
#include <stdlib.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


const int STEP = 10;
double weight[STEP];

#define AVERAGE_WEIGHT            0
#define LINEARLY_CHANGINE_WEIGHT  1
#define GAUSSIAN_WEIGHTING        2
#define EXPONENTIAL_WEIGHTING     3


void init_weight(int flag)
{
    if(flag == AVERAGE_WEIGHT)
    {
        for(int i = 0; i < STEP; i++)
            weight[i] = 1.0/STEP;
    }

    else if(flag == LINEARLY_CHANGINE_WEIGHT)
    {
        double sum = (1+STEP)*STEP/2;

        for(int i = 0; i < STEP; i++)
            weight[i] = (i+1)/sum;
    }

    else if(flag == GAUSSIAN_WEIGHTING)
    {
        double u = 5.5;
        double o = 1.0;

        double sum = 0;

        for(int i = 0; i < STEP; i++)
        {
            weight[i] = 1-exp(pow(i+1-u, 2)/o);
            sum += weight[i];
        }

        for(int i = 0; i < STEP; i++)
            weight[i] /= sum;
    }

    else if(flag == EXPONENTIAL_WEIGHTING)
    {
        double lamda = 1.65;
        double sum = 0;

        for(int i = 0; i < STEP ; i++)
        {
            weight[i] = pow(lamda, i+1);
            sum += weight[i];
        }

        for(int i = 0; i < STEP; i++)
            weight[i] /= sum;
    }
}


void push_to_buffer(uchar *buffer, int width, int height, cv::Mat &frame)
{
    cv::Mat tmpImg;

    if(frame.channels() == 3)
        cv::cvtColor(frame, tmpImg, cv::COLOR_BGR2GRAY);

    else if(frame.channels() == 1)
        tmpImg = frame;

    else return;

    int rows = tmpImg.rows;
    int cols = tmpImg.cols;
    int step = tmpImg.step;

    assert(width == cols && height == rows);

    for(int y = 0; y < rows; y++)
        memcpy(buffer + y*cols, tmpImg.data + y*step, sizeof(uchar) * cols);
}


void merge_frame(uchar **buffer, uchar *dst, int width, int height, int size)
{
    memset(dst, 0, sizeof(uchar) * width *height);

    float *mergeImg = (float*)malloc(width * height *sizeof(float));
    memset(mergeImg, 0, sizeof(float) * width * height);

    for(int i = 0; i < size; i++)
    {
        for(int y = 0; y < height; y++)
            for(int x = 0; x < width; x++)
                mergeImg[y*width+x] += weight[i] * buffer[i][y*width+x];
    }

    for(int y = 0; y < height; y++)
        for(int x = 0; x < width; x++)
            dst[y*width+x] = mergeImg[y*width+x];

    free(mergeImg);
}


int main(int argc, char **argv)
{
    if(argc<3)
    {
        printf("Usage: %s [video name] [out dir]\n", argv[0]);
        return 1;
    }

    cv::VideoCapture cap(argv[1]);
    if(!cap.isOpened())
    {
        printf("Can't open video %s\n", argv[1]);
        return 0;
    }

    char *outdir = argv[2];

    if(outdir[strlen(outdir)-1] == '/')
        outdir[strlen(outdir)-1] = '\0';

//    init_weight(AVERAGE_WEIGHT);
    init_weight(EXPONENTIAL_WEIGHTING);

    int  width = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int  height = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    long totalFrameNumber = (long)cap.get(CV_CAP_PROP_FRAME_COUNT);

    totalFrameNumber = totalFrameNumber - totalFrameNumber % STEP;

    uchar *frameBuffer = (uchar*)malloc(sizeof(uchar) * width * height * (STEP+1));
    uchar *pbuffer[STEP+1];

    for(int i = 0; i < STEP+1; i++)
        pbuffer[i] = frameBuffer + i * width * height;

    cv::Mat frame;

    for(long frameNumber = 0; frameNumber < totalFrameNumber; frameNumber += STEP)
    {
        for(int i = 0; i < STEP; i++)
        {
            cap >> frame;
            assert(!frame.empty());
            push_to_buffer(pbuffer[i], width, height, frame);
        }

        merge_frame(pbuffer, pbuffer[STEP], width, height, STEP);

        cv::Mat img(height, width, CV_8UC1, pbuffer[STEP]);

        char fileName[128];

        sprintf(fileName, "%s/%ld.jpg", outdir, frameNumber);

        if(!cv::imwrite(fileName, img))
        {
            printf("Can't write image %s\n", fileName);
        }

        printf("%6.2lf%%\r", 100.0*(frameNumber+STEP)/totalFrameNumber);
        fflush(stdout);
    }

    printf("\n");

    free(frameBuffer);

    return 0;
}
