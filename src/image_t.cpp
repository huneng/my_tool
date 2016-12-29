#include "tool.h"


int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [image list] [outdir]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imgList;
    read_file_list(argv[1], imgList);

    int size = imgList.size();
    char rootDir[256], fileName[128], ext[20], outPath[256];

    for(int i = 0; i < size; i++){
        cv::Mat img = cv::imread(imgList[i], 0);

        if(img.empty()){
            printf("Can't open image %s\n", imgList[i].c_str());
            break;
        }

        img = img.t();
        analysis_file_path(imgList[i].c_str(), rootDir, fileName, ext);
        sprintf(outPath, "%s/%s_t.%s", argv[2], fileName, ext);

        if(!cv::imwrite(outPath, img)){
            printf("Can't write image %s\n", outPath);
            break;
        }

        printf("%6.2f%%\r", 100.0 * (i+1) / size);fflush(stdout);
    }


    return 0;
}
