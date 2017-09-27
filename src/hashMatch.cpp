#include "tool.h"

#define N 32
#define N2 8
#define PI 3.1415926
#define LEN 64


double dctcoef1[N][N];
double dctcoef2[N][N];


int hamdist(uint32_t *a, uint32_t *b, int len);
void print_bin(uint32_t code);

void init_coef()
{
    double coef[N];

    for(int i = 0; i < N; i++)
        coef[i] = 1;

    coef[0] = 1/sqrt(2.0);

    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++)
        {
            dctcoef1[i][j] = cos( (2 * i + 1) / (2.0 * N) * j * PI);
            dctcoef2[i][j] = coef[i] * coef[j]/4.0;
        }
    }
}


void dct(double *data, double *dst)
{
    for(int u = 0; u < N2; u++)
    {
        for(int v = 0; v < N2; v++)
        {
            double sum = 0.0;

            for(int i = 0; i < N; i++)
                for(int j = 0; j < N; j++)
                    sum += dctcoef1[i][u] * dctcoef1[j][v] * data[i * N + j];

            dst[u * N2 + v] = sum * dctcoef2[u][v];
        }
    }
}


void gen_hash_code(cv::Mat& img, uint32_t* hashCode)
{
    cv::Mat gray, smallImg;

    double *data, *dctres;
    double avg = 0;

    int y, x;
    int i;

    if(img.type() == CV_8UC3)
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    else if(img.type() == CV_8UC1)
        img.copyTo(gray);

    cv::resize(gray, smallImg, cv::Size(N, N));

    data = new double[N * N];

    for(y = 0; y < N; y++)
        for(x = 0; x < N; x++)
            data[y * N + x] = smallImg.at<uchar>(y, x);


    dctres = new double[N2 * N2];

    dct(data, dctres);

    int size = N2 * N2;

    for(int i = 1; i < size; i++){
        avg += dctres[i];
    }

    avg = avg/(size - 1);

    uint32_t hashZero = 0x00000000;
    uint32_t hashOne  = 0x00000001;
    uint32_t code;

    int half = size >> 1;

    for(int i = 1; i < half ; i++)
    {
        code = dctres[i] > avg;
        hashZero <<= 1;
        hashZero |= code;
    }

    hashCode[0] = hashZero;

    hashZero = 0x00000000;

    for(i = half; i < size; i++)
    {
        code = dctres[i] > avg;
        hashZero <<= 1;
        hashZero |= code;
    }

    hashCode[1] = hashZero;

    delete [] dctres;
    delete [] data;
}


typedef struct {
    int id;
    uint16_t key;
    uint64_t code;
    char path[256];
} Pair;

#define LT(a, b) ((a).key < (b).key ? 1 : ((a).key == (b).key ? ((a).id < (b).id) : 0) )

HU_IMPLEMENT_QSORT(sort_arr_pairs, Pair, LT);

int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [image list] [out file]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imgList;
    int size;

    FILE *fout;

    read_file_list(argv[1], imgList);

    size = imgList.size();

    if(size == 0) return 0;

    fout = fopen(argv[2], "w");

    if(fout == NULL){
        printf("Can't open file %s\n", argv[2]);
        return 1;
    }

    init_coef();

    Pair *pairs = new Pair[size];

    memset(pairs, 0, sizeof(Pair) * size);

    for(int i = 0; i < size; i++){
        const char *imgPath = imgList[i].c_str();

        cv::Mat img = cv::imread(imgPath, 0);
        Pair *pair = pairs + i;

        if(img.empty()){
            printf("Can't open image %s\n", imgPath);
            return 1;
        }

        pair->id = i;
        strcpy(pair->path, imgPath);

        gen_hash_code(img, (uint32_t*)(&pair->code));
        pair->key = uint16_t(pair->code & 0xffff);

        printf("%d\r", i), fflush(stdout);
    }

    sort_arr_pairs(pairs, size);

    uint16_t startKey = pairs[0].key;
    int startId = 0;

    for(int i = 1; i < size; i++){
        if(startKey != pairs[i].key){
            startKey = pairs[i].key;
            startId = i;
            continue;
        }

        for(int j = startId; j < i; j++){
            int dist = hamdist((uint32_t*)(&pairs[j].code), (uint32_t*)(&pairs[i].code), 2);
            if(dist > 0) continue;

            fprintf(fout, "%s %s\n", pairs[j].path, pairs[i].path);
            fflush(fout);
        }

        printf("%d\r", i), fflush(stdout);
    }

    fclose(fout);


    return 0;
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


// compute Hamming Distance between two binary streams
int hamdist(uint32_t *a, uint32_t *b, int len)
{
    int i;
    int dist = 0;
    uint32_t mask = 0x000000ff;

    for(i = 0; i < len; i++)
    {
        uint32_t res = a[i] ^ b[i];

        dist += bit_count[(res >> 24) & mask] +
            bit_count[(res >> 8) & mask] +
            bit_count[res & mask];
    }

    return dist;
}


void print_bin(uint32_t code)
{
    uint32_t hashOne = 0x00000001;

    char out[33];

    for(int i = 31; i >= 0 ; i--)
    {
        out[i] = (code & hashOne) + '0';
        code >>= 1;
    }

    out[32] = '\0';
    printf("%s ", out);
}
