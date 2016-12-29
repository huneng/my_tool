#include "tool.h"
#include "face_tool.h"

int main(int argc, char **argv){
    if(argc < 4){
        printf("Usage: %s [image list] [flag] [out dir]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imgList;
    int size;
    int id;
    int *idxs;
    int ptsSize;


    std::vector<Sample*> samples;

    printf("READ SAMPLES\n");
    if(read_samples(argv[1], samples, 96 * 3) != 0)
        return 2;

    id = atoi(argv[2]);

    if(id == 0){
        printf("POINT SIZE = 68\n");

        return 2;
    }
    else if(id == 1){
        printf("POINT SIZE = 51\n");
        ptsSize = 51;

        idxs = new int[ptsSize];

        for(int i = 0; i < ptsSize; i++)
            idxs[i] = 17 + i;
    }
    else if(id == 2){
        printf("POINT SIZE = 43\n");
        ptsSize = 43;

        idxs = new int[ptsSize];

        for(int i = 0; i < ptsSize; i++)
            idxs[i] = 17 + i;
    }

    for(int i = 0; i < ptsSize; i++)
        printf("%d ", idxs[i]);
    printf("\n");

    size = samples.size();

    char outFile[256];

    printf("CREATE SAMPLES\n");
    for(int i = 0; i < size; i++){
        Sample *sample = samples[i];

        Shape shape(ptsSize);
        int oriSize = sample->shape.size();

        for(int j = 0; j < ptsSize; j++){
            shape[j] = sample->shape[idxs[j]];
        }

        sprintf(outFile, "%s/%s.jpg", argv[3], sample->imgName.c_str());
        cv::imwrite(outFile, sample->img);

        sprintf(outFile, "%s/%s.pts", argv[3], sample->imgName.c_str());
        write_pts_file(outFile, shape);
    }

    delete [] idxs;

    return 0;
}
