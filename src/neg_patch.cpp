#include "tool.h"

#define WIN_SIZE 64
#define LEN 8
#define MAX_COUNT 500000
#define DIST_THRESH 6

uint64_t generate_hash_key(uint8_t *img, int stride){
    int dsize = WIN_SIZE  / LEN;
    int dsize2 = dsize * dsize;

    uint32_t sum[64];

    memset(sum, 0, sizeof(uint32_t) * 64);

    for(int y = 0; y < WIN_SIZE; y++){
        int iy = y / LEN * dsize;

        for(int x = 0; x < WIN_SIZE; x++){
            int ix = x / LEN;
            sum[iy + ix] += img[y * stride + x];
        }
    }

    uint32_t mean = 0;

    for(int i = 0; i < dsize2; i++)
        mean += sum[i];

    mean /= dsize2;


    uint64_t code = 0;

    for(int i = 0; i < dsize2; i++){
        code <<= 1;
        code |= (sum[i] > mean);
    }

    return code;
}


const unsigned char bit_count[256] =
{
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};


int hamdist(uint32_t *a, uint32_t *b, int len)
{
    int i;
    int dist = 0;
    uint32_t mask = 0x000000ff;

    for(i = 0; i < len; i++){
        uint32_t res = a[i] ^ b[i];

        dist += bit_count[(res >> 24) & mask] +
            bit_count[(res >> 8) & mask] +
            bit_count[res & mask];
    }

    return dist;
}


int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [image list] [out file]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imgList;
    int isize;

    read_file_list(argv[1], imgList);
    isize = imgList.size();

    FILE *fout = fopen(argv[2], "wb");

    if(fout == NULL){
        printf("Can't open file %s\n", argv[2]);
        return 2;
    }

    int DSIZE = WIN_SIZE / LEN;
    int cum = 0;
    int cum2 = 0;


    uint64_t *idx = new uint64_t[MAX_COUNT * 2]; assert(idx != NULL);

    memset(idx, 0, sizeof(uint64_t) * MAX_COUNT * 2);

    for(int cycle = 0; cycle < 10; cycle ++){

        printf("cycle = %d       \n", cycle);

        int dx = WIN_SIZE * 0.4;
        int dy = WIN_SIZE * 0.4;

        int rsize = (cycle + 1) * WIN_SIZE;

        for(int i = 0; i < isize; i++){
            char rootDir[256], fileName[256], ext[30], filePath[256];

            const char *imgPath = imgList[i].c_str();

            cv::Mat img = cv::imread(imgPath, 0);

            if(img.empty()){
                printf("Can't open image %s\n", imgPath);
                continue;
            }

            analysis_file_path(imgPath, rootDir, fileName, ext);

            if(img.cols > img.rows)
                cv::resize(img, img, cv::Size(rsize * img.cols / img.rows, rsize));
            else
                cv::resize(img, img, cv::Size(rsize, rsize * img.rows / img.cols));

            for(int y = 0; y <= img.rows - WIN_SIZE; y += dy){
                for(int x = 0; x <= img.cols - WIN_SIZE; x += dx){
                    uint8_t *ptr = img.data + y * img.step + x;

                    uint64_t key = generate_hash_key(ptr, img.step) + 1;
                    int flag = 1;

                    for(int k = 0; k < cum; k++){
                        if(hamdist((uint32_t*)(&key), (uint32_t*)(idx + k), 2) < DIST_THRESH){
                            flag = 0;
                            cum2 ++;
                            break;
                        }
                    }

                    if(flag == 1){
                        sprintf(filePath, "%s_%d_%d_%d.jpg", fileName, rsize, y, x);

                        int ret;

                        ret = fwrite(filePath, sizeof(char), 255, fout);assert(ret == 255);

                        for(int iy = 0; iy < WIN_SIZE; iy++){
                            ret = fwrite(ptr + iy * img.step, sizeof(uint8_t), WIN_SIZE, fout);
                            assert(ret == WIN_SIZE);
                        }

                        idx[cum] = key;
                        cum ++;
                    }
                }
            }

            if(i % 10000 == 0)
                printf("%d %d %d\r", cum, cum2, i), fflush(stdout);
        }

        fflush(fout);

        if(cum >= MAX_COUNT )
            break;
    }

    fclose(fout);

    return 0;
}
