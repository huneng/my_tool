#include <stdlib.h>
#include "tool.h"

const int MAX_SIZE = 1680;

int main(int argc, char **argv){
    if(argc < 2){
        printf("Usage: %s [image list]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imageList;
    read_file_list(argv[1], imageList);

    int size = imageList.size();
    char winName[128];
    for(int i = 0; i < size; i++){
        cv::Mat img = cv::imread(imageList[i], 1);
        if(img.empty())
            continue;

        printf("%s                                 \r", imageList[i].c_str());
        fflush(stdout);
        cv::imshow("img", img);
        cv::waitKey();

        cv::destroyWindow(winName);
    }

    return 0;
}
