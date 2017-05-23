#include "tool.h"


int main(int argc, char **argv){
    if(argc < 2){
        printf("Usage: %s [image dir]\n", argv[0]);
        return 1;
    }

    int ret;
    char command[256];
    std::vector<std::string> imgList;
    int size;
    char key;

    sprintf(command, "ls -v %s/*.jpg > list.txt", argv[1]);
    ret = system(command);

    read_file_list("list.txt", imgList);

    size = imgList.size();
    if(size == 0) return 0;

    int id = 0;

    if(argc == 3){
        id = atoi(argv[2]);
        id = HU_MIN(id, size - 1);
        id = HU_MAX(id, 0);
    }

    while(key != 'q'){
        cv::Mat img = cv::imread(imgList[id], 1);
        printf("%s\r", imgList[id].c_str()), fflush(stdout);
        if(img.cols > 720)
            cv::resize(img, img, cv::Size(720, img.rows * 720 / img.cols));

        if(img.rows > 720)
            cv::resize(img, img, cv::Size(img.cols * 720 / img.rows, 720));

        cv::imshow("img", img);
        key = cv::waitKey();

        if(key == 'd'){
            sprintf(command, "rm %s", imgList[id].c_str());
            ret = system(command);

            sprintf(command, "ls -v %s/*.jpg > list.txt", argv[1]);
            ret = system(command);

            imgList.clear();
            read_file_list("list.txt", imgList);
            size = imgList.size();
            if(size == 0) break;
        }
        else if(key == 'N'){
            id --;
        }
        else if(key == 'n'){
            id ++;
        }
        else {
            continue;
        }

        id = HU_MAX(id, 0);
        id = HU_MIN(id, size - 1);
    }

    return 0;
}
