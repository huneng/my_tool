#include "tool.h"

#include <stdlib.h>


const int MAX_SIZE = 1680;


void merge_show(const char *imgPath1, const char *imgPath2){
    cv::Mat img1 = cv::imread(imgPath1, 1);
    cv::Mat img2 = cv::imread(imgPath2, 1);
    cv::Mat res;

    if(img1.empty() || img2.empty()){
        printf("Can't open image\n");
        return ;
    }

    int cols = img1.cols + img2.cols;
    int rows = img1.rows > img2.rows ? img1.rows : img2.rows;

    res = cv::Mat::zeros( rows, cols,img1.type());

    cv::Mat left(res, cv::Rect(0, 0, img1.cols, img1.rows));
    cv::Mat right(res, cv::Rect(img1.cols, 0, img2.cols, img2.rows));

    left += img1;
    right += img2;


    if(res.cols > MAX_SIZE)
        cv::resize(res, res, cv::Size(MAX_SIZE, MAX_SIZE * res.rows / res.cols));

    cv::imshow("img", res);
    cv::waitKey();
}


int main(int argc, char **argv){
    if(argc < 4){
        printf("Usage: %s [flag] [iterm 1] [iterm 2]\n", argv[0]);
        return 1;
    }

    int flag = atoi(argv[1]);

    if(flag == 1){
        std::vector<std::string> list1, list2;

        read_file_list(argv[2], list1);
        read_file_list(argv[3], list2);

        int size = list1.size() < list2.size() ? list1.size() : list2.size();

        for(int i = 0; i < size; i++){
            merge_show(list1[i].c_str(), list2[i].c_str());
        }
    }
    else {
        merge_show(argv[2], argv[3]);
    }
    return 0;
}
