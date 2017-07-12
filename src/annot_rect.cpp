#include "tool.h"
#include "face_tool.h"

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

                cv::rectangle(img, ptr->rect, cv::Scalar(0, 255, 0), 1);
                cv::imshow(WIN_NAME, img);
                if(event == CV_EVENT_LBUTTONUP)
                    ptr->flag = 0;
            }

            break;
    }
}


int rect_images(cv::Mat &img, std::vector<cv::Rect> &rects){
    cv::namedWindow(WIN_NAME);
    AnnotParams params;

    img.copyTo(params.img);

    cv::setMouseCallback(WIN_NAME, on_mouse, &params);

    char key = 'q';

    rects.clear();

    while(1){
        params.flag = 0;
        memset(&params.rect, 0, sizeof(cv::Rect));

        cv::imshow(WIN_NAME, params.img);

        key = cv::waitKey();
        if(key == 'n' && params.rect.width > 10){
            if(rects.size() > 0 && rects.back() == params.rect)
                continue;

            cv::rectangle(params.img, params.rect, cv::Scalar(0, 255, 0), 2);
            rects.push_back(params.rect);
        }
        else if(key == 'q' ) {
            break;
        }
    }
}

int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [image list] [out dir]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imgList;

    char rootDir[128], fileName[128], ext[30], filePath[256];
    int size;

    FILE *fout;
    read_file_list(argv[1], imgList);

    size  = imgList.size();


    for(int i = 0; i < size; i++){
        const char *imgPath = imgList[i].c_str();
        cv::Mat src = cv::imread(imgPath, 1);
        cv::Mat img;

        if(src.empty()) continue;

        std::vector<cv::Rect > rects;

        float scale1 = 1.0f;
        float scale2 = 1.0f;
        float scale;

        analysis_file_path(imgPath, rootDir, fileName, ext);

        src.copyTo(img);
        if(img.cols > 720){
            scale1 = img.cols / 720.0f;
            cv::resize(img, img, cv::Size(720, img.rows * 720 / img.cols));
        }

        if(img.rows > 720){
            scale2 = img.rows / 720.0f;
            cv::resize(img, img, cv::Size(img.cols * 720 / img.rows, 720));
        }

        scale = scale1 * scale2;

        rect_images(img, rects);

        int rsize = rects.size();

        printf("%d\n", rsize);
        for(int j = 0; j < rsize; j++){
            cv::Rect rect = rects[j];

            rect.x *= scale;
            rect.y *= scale;
            rect.width *= scale;
            rect.height *= scale;

            rect.x -= rect.width * 0.5;
            rect.y -= rect.height * 0.5;
            rect.width *= 2;
            rect.height *= 2;

            cv::Mat patch;

            /*
            assert(rect.x >= 0 && rect.y >= 0 && rect.x + rect.width < src.cols && rect.y + rect.height < src.rows);

            cv::rectangle(src, rect, cv::Scalar(0, 255, 0), 2);
            cv::imshow("src", src);
            cv::waitKey();
            //*/

            //*
            extract_area_from_image(src, patch, rect);

            sprintf(filePath, "%s/%s_%d.jpg", argv[2], fileName, j);
            if(!cv::imwrite(filePath, patch)){
                printf("Can't write image %s\n", filePath);
            }
            //*/
        }

        rects.clear();

        printf("%s %d\n", fileName, size - i);
    }


    return 0;
}
