#include "tool.h"
#include "face_tool.h"


#define WIN_SIZE 128

int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [image list] [outdir]\n", argv[0]);
        return 0;
    }

    std::vector<std::string> imgList;

    read_file_list(argv[1], imgList);
    int size = imgList.size();

    char rootDir[256], fileName[256], ext[30], filePath[256];

    for(int i = 0; i < size; i++){
        cv::Mat patch;
        cv::Mat img = cv::imread(imgList[i], 1);
        Shape shape;

        if(img.empty()){
            printf("Can't open image %s\n", imgList[i].c_str());
            break;
        }

        analysis_file_path(imgList[i].c_str(), rootDir, fileName, ext);

        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        read_pts_file(filePath, shape);

        normalize_sample(img, patch, WIN_SIZE * 3, 3.0, shape);

        save_sample(patch, shape, argv[2], fileName);

        printf("%.2f%%\r", 100.0 * (i+1)/ size);
        fflush(stdout);
    }


    return 0;
}
