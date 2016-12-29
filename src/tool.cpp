#include "tool.h"


int read_file_list(const char *filePath, std::vector<std::string> &fileList)
{
    char line[512];
    FILE *fin = fopen(filePath, "r");

    if(fin == NULL)
    {
        printf("Can't open file: %s\n", filePath);
        return -1;
    }

    while(fscanf(fin, "%s\n", line) != EOF)
    {
        fileList.push_back(line);
    }

    fclose(fin);

    return 0;
}


void analysis_file_path(const char* filePath, char *rootDir, char *fileName, char *ext)
{
    int len = strlen(filePath);
    int idx = len - 1, idx2 = 0;

    while(idx >= 0)
    {
        if(filePath[idx] == '.')
            break;
        idx--;
    }

    if(idx >= 0){
        strcpy(ext, filePath + idx + 1);
        ext[len - idx] = '\0';
    }
    else {
        ext[0] = '\0';
        idx = len - 1;
    }

    idx2 = idx;
    while(idx2 >= 0){
#if defined(WIN32)
        if(filePath[idx2] == '\\')
#elif defined(linux)
        if(filePath[idx2] == '/')
#endif
            break;
        idx2 --;
    }

    if(idx2 > 0){
        strncpy(rootDir, filePath, idx2);
        rootDir[idx2] = '\0';
    }
    else{
        rootDir[0] = '.';
        rootDir[1] = '\0';
    }

    strncpy(fileName, filePath + idx2 + 1, idx - idx2 - 1);
    fileName[idx-idx2-1] = '\0';
}


void npd_normalize(uint8_t *img, int width, int height, int stride){
    cv::RNG rng(cv::getTickCount());

    double sum[3] = {0, 0, 0};

    uint8_t *bk = img;
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            uint8_t *ptr = img + x * 3;
            sum[0] += ptr[0];
            sum[1] += ptr[1];
            sum[2] += ptr[2];
        }

        img += stride;
    }

    sum[0] /= (width * height);
    sum[1] /= (width * height);
    sum[2] /= (width * height);

    img = bk;
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            uint8_t *ptr = img + x * 3;
            ptr[0] = uint8_t(((ptr[0] - sum[0]) / (ptr[0] + sum[0]) + 1.0) / 2 * 255);
            ptr[1] = uint8_t(((ptr[1] - sum[1]) / (ptr[1] + sum[1]) + 1.0) / 2 * 255);
            ptr[2] = uint8_t(((ptr[2] - sum[2]) / (ptr[2] + sum[2]) + 1.0) / 2 * 255);
        }

        img += stride;
    }
}


void noise_image(uint8_t *img, int width, int height, int stride){
    cv::RNG rng(cv::getTickCount());
    int RANGE = 16;

    for(int y = 0; y < height; y++){
        for(int x = 0; x < stride; x++){
            if(img[x] >= RANGE && img[x] <= 255 - RANGE)
                img[x] += rng.uniform(-RANGE, RANGE);
        }

        img += stride;
    }
}


void inverse_color(uint8_t *img, int width, int height, int stride){
    for(int y = 0; y < height; y++){
        for(int x = 0; x < stride; x++)
            img[x] = 255 - img[x];
        img += stride;
    }
}


void mirror_image(uint8_t *img, int width, int height, int stride){
    int cx = width / 2;

    if(stride / width == 1){
        for(int y = 0; y < height; y++){
            for(int x = 0; x < cx; x++)
                HU_SWAP(img[x], img[width - 1 - x], uint8_t);
            img += stride;
        }
    }
    else if(stride / width >= 3){
        for(int y = 0; y < height; y++){
            for(int x = 0; x < cx; x++){
                int z1 , z2;
                z1 = x * 3;
                z2 = (width - 1 - x) * 3;

                HU_SWAP(img[z1], img[z2], uint8_t);

                z1++; z2++;
                HU_SWAP(img[z1], img[z2], uint8_t);

                z1++; z2++;
                HU_SWAP(img[z1], img[z2], uint8_t);
            }

            img += stride;
        }
    }
}


cv::Point2f calculate_central(std::vector<cv::Point2f> &shape){
    int ptsSize = shape.size();

    double cx = 0;
    double cy = 0;

    for(int i = 0; i < ptsSize; i++){
        cx += shape[i].x;
        cy += shape[i].y;
    }

    return cv::Point2f(cx / ptsSize, cy / ptsSize);
}


