#include "tool.h"
#include "face_tool.h"


#define WIN_SIZE 800

typedef struct{
    char winName[30];
    cv::Mat img;
    cv::Point2f pt;
} SImage;


void on_mouse(int event, int x, int y, int flags, void *params){
    SImage *simg = (SImage*)params;

    switch(event){
        case CV_EVENT_LBUTTONDOWN:
            simg->pt.x = x;
            simg->pt.y = y;

            {
                cv::Mat timg;
                simg->img.copyTo(timg);
                cv::circle(timg, simg->pt, 2, cv::Scalar(0, 255, 0), -1);
                cv::imshow(simg->winName, timg);
            }
            break;

    }
}


void annot_image(cv::Mat &img, Shape &shape){
    int ptsSize = shape.size();
    SImage simg;
    sprintf(simg.winName, "img");
    cv::namedWindow(simg.winName);

    cv::setMouseCallback(simg.winName, on_mouse, &simg);

    char key = 'n';
    while(1){
        img.copyTo(simg.img);

        for(int p = 0; p < ptsSize; p++){
            cv::circle(simg.img, shape[p], 2, cv::Scalar(0, 0, 255), -1);
            char str[4];
            sprintf(str, "%d", p);
            cv::putText(simg.img, str, cv::Point(shape[p].x + 5, shape[p].y), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(255, 0, 0));
        }

        cv::imshow(simg.winName, simg.img);
        key = cv::waitKey(100);

        if(key == 'q')
            break;

        printf("POINT NO: \n");
        int no ;

        int ret = scanf("%d", &no);

        printf("%d\n", no);
        if(no < 0) break;

        simg.pt.x = shape[no].x;
        simg.pt.y = shape[no].y;

        key = cv::waitKey();

        shape[no].x = simg.pt.x;
        shape[no].y = simg.pt.y;

        if(key == 'q')
            break;
    }
}


int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [image list] [out dir]\n", argv[0]);
        return 0;
    }

    std::vector<std::string> imgList;
    char rootDir[128], fileName[128], ext[30], filePath[256];
    int size = 0;

    read_file_list(argv[1], imgList);

    size = imgList.size();

    FILE *fout = fopen("not_list.txt", "a");
    for(int i = 0; i < size; i++){
        std::string imgPath = imgList[i];

        cv::Mat img = cv::imread(imgPath, 1);

        analysis_file_path(imgPath.c_str(), rootDir, fileName, ext);

        printf("%d %s\n", size - i, fileName);

        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        Shape shape;

        read_pts_file(filePath, shape);

        int ptsSize = shape.size();

        float minx = FLT_MAX, maxx = -FLT_MAX;
        float miny = FLT_MAX, maxy = -FLT_MAX;

        for(int p = 0; p < ptsSize; p++){
            minx = HU_MIN(minx, shape[p].x);
            maxx = HU_MAX(maxx, shape[p].x);
            miny = HU_MIN(miny, shape[p].y);
            maxy = HU_MAX(maxy, shape[p].y);
        }

        float faceSize = HU_MAX(maxx - minx + 1, maxy - miny + 1);

        float scale = faceSize / WIN_SIZE;

        cv::Rect rect;

        rect.x = (minx + maxx) * 0.5f - faceSize * 0.6f;
        rect.y = (miny + maxy) * 0.5f - faceSize * 0.6f;

        faceSize *= 1.2f;

        rect.width  = faceSize;
        rect.height = faceSize;

        if(rect.x < 0 || rect.y < 0 || rect.x + rect.width > img.cols || rect.y + rect.height > img.rows){
            fprintf(fout, "%s\n", imgPath.c_str());
            fflush(fout);
            continue;
        }

        cv::Mat sImg(img, rect);

        cv::resize(sImg, sImg, cv::Size(sImg.cols / scale, sImg.rows / scale));

        for(int p = 0; p < ptsSize; p++){
            shape[p].x = (shape[p].x - rect.x) / scale;
            shape[p].y = (shape[p].y - rect.y) / scale;
        }

        annot_image(sImg, shape);

        sprintf(filePath, "%s/%s.jpg", argv[2], fileName);
        cv::imwrite(filePath, img);

        for(int i = 0; i < ptsSize; i++){
            shape[i].x = shape[i].x * scale + rect.x;
            shape[i].y = shape[i].y * scale + rect.y;
        }

        sprintf(filePath, "%s/%s.pts", argv[2], fileName);
        write_pts_file(filePath, shape);
    }

    fclose(fout);

    return 0;
}


