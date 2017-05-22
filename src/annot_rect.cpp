#include "tool.h"


#define WIN_NAME "IMG"


typedef struct {
    cv::Mat img;
    cv::Rect rect;
    int flag;
} AnnotParams;

void on_mouse(int event, int x, int y, int flags, void *params){
    AnnotParams *ptr = (AnnotParams*)params;

    cv::Mat img;
    ptr->img.copyTo(img);

    switch(event){
        case CV_EVENT_LBUTTONDOWN:
            ptr->rect.x = x;
            ptr->rect.y = y;
            ptr->flag = 1;
            break;

        case CV_EVENT_MOUSEMOVE:
        case CV_EVENT_LBUTTONUP:

            if(ptr->flag == 1){
                int rectSize = HU_MAX(x - ptr->rect.x + 1, y - ptr->rect.y + 1);

                ptr->rect.width  = rectSize;
                ptr->rect.height = rectSize;

                cv::rectangle(img, ptr->rect, cv::Scalar(0, 0, 255), 1);
                cv::imshow(WIN_NAME, img);
                if(event == CV_EVENT_LBUTTONUP)
                    ptr->flag = 0;
            }

            break;
    }
}

void rect_image(cv::Mat &img, cv::Rect &res){
    cv::namedWindow(WIN_NAME);
    AnnotParams params;
    params.img = img;

    params.flag = 0;
    cv::setMouseCallback(WIN_NAME, on_mouse, &params);

    cv::imshow(WIN_NAME, params.img);
    char key = cv::waitKey();

    res = params.rect;
}


int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [image list] [out file]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imgList;

    char rootDir[128], fileName[128], ext[30], filePath[256];
    int size;

    FILE *fout;
    read_file_list(argv[1], imgList);

    size  = imgList.size();

    fout = fopen(argv[2], "a");

    for(int i = 0; i < size; i++){
        const char *imgPath = imgList[i].c_str();
        cv::Mat img = cv::imread(imgPath, 1);
        cv::Rect rect;


        float scale1 = 1.0f;
        float scale2 = 1.0f;
        float scale;

        if(img.cols > 720){
            scale1 = img.cols / 720.0f;
            cv::resize(img, img, cv::Size(720, img.rows * 720 / img.cols));
        }

        if(img.rows > 720){
            scale2 = img.rows / 720.0f;
            cv::resize(img, img, cv::Size(img.cols * 720 / img.rows, 720));
        }

        scale = scale1 * scale2;

        rect_image(img, rect);
        rect.x *= scale;
        rect.y *= scale;
        rect.width *= scale;
        rect.height *= scale;

        printf("%s %d %d %d %d\n", imgPath, rect.x, rect.y, rect.width, rect.height);
        fprintf(fout, "%s %d %d %d %d\n", imgPath, rect.x, rect.y, rect.width, rect.height);
        fflush(fout);
    }
    fclose(fout);


    return 0;
}
