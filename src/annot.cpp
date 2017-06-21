#include "tool.h"
#include "face_tool.h"


#define WIN_SIZE 600

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
    int ptsSize = shape.ptsSize;
    SImage simg;
    sprintf(simg.winName, "img");
    cv::namedWindow(simg.winName);

    cv::setMouseCallback(simg.winName, on_mouse, &simg);

    char key = 'n';
    while(1){
        img.copyTo(simg.img);

        for(int p = 0; p < ptsSize; p++){
            cv::circle(simg.img, cv::Point2f(shape.pts[p].x, shape.pts[p].y), 2, cv::Scalar(0, 0, 255), -1);
            char str[4];
            sprintf(str, "%d", p);
            cv::putText(simg.img, str, cv::Point(shape.pts[p].x + 5, shape.pts[p].y), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(255, 0, 0));
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

        simg.pt.x = shape.pts[no].x;
        simg.pt.y = shape.pts[no].y;

        key = cv::waitKey();

        shape.pts[no].x = simg.pt.x;
        shape.pts[no].y = simg.pt.y;

        if(key == 'q')
            break;
    }
}


int initialize_points(Shape &shape, int width, int height){
    printf("PLEASE INPUT POINT SIZE: ");fflush(stdout);

    int ret = scanf("%d", &shape.ptsSize);

    if(shape.ptsSize < 4 )
        return 1;

    int faceSize = HU_MIN(width, height) * 0.9;
    int radius = faceSize >> 1;
    int cx = width >> 1;
    int cy = height >> 1;

    shape.pts[0].x = cx - radius;
    shape.pts[0].y = cy - radius;

    shape.pts[1].x = cx - radius;
    shape.pts[1].y = cy + radius;

    shape.pts[2].x = cx + radius;
    shape.pts[2].y = cy + radius;

    shape.pts[3].x = cx + radius;
    shape.pts[3].y = cy - radius;

    for(int i = 4; i < shape.ptsSize; i++){
        shape.pts[i].x = cx;
        shape.pts[i].y = cy;
    }

    return 0;
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

    for(int i = 0; i < size; i++){
        std::string imgPath = imgList[i];

        cv::Mat img = cv::imread(imgPath, 1);

        analysis_file_path(imgPath.c_str(), rootDir, fileName, ext);

        printf("%d %s\n", size - i, fileName);

        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        Shape shape;

        if(read_pts_file(filePath, shape) == 0){
            printf("Can't read pts file %s\n", filePath);
            if( initialize_points(shape, img.cols, img.rows) )
                continue;
        }

        int ptsSize = shape.ptsSize;

        float minx = FLT_MAX, maxx = -FLT_MAX;
        float miny = FLT_MAX, maxy = -FLT_MAX;

        for(int p = 0; p < ptsSize; p++){
            minx = HU_MIN(minx, shape.pts[p].x);
            maxx = HU_MAX(maxx, shape.pts[p].x);
            miny = HU_MIN(miny, shape.pts[p].y);
            maxy = HU_MAX(maxy, shape.pts[p].y);
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
            int border = faceSize * 0.1f;
            printf("make border : %d\n", border);
            cv::copyMakeBorder(img, img, border, border, border, border, cv::BORDER_CONSTANT);

            rect.x += border;
            rect.y += border;

            for(int p = 0; p < ptsSize; p++){
                shape.pts[p].x += border;
                shape.pts[p].y += border;
            }
        }

        cv::Mat sImg(img, rect);

        cv::resize(sImg, sImg, cv::Size(sImg.cols / scale, sImg.rows / scale));

        for(int p = 0; p < ptsSize; p++){
            shape.pts[p].x = (shape.pts[p].x - rect.x) / scale;
            shape.pts[p].y = (shape.pts[p].y - rect.y) / scale;
        }

        annot_image(sImg, shape);

        sprintf(filePath, "%s/%s.jpg", argv[2], fileName);
        cv::imwrite(filePath, img);

        for(int i = 0; i < ptsSize; i++){
            shape.pts[i].x = shape.pts[i].x * scale + rect.x;
            shape.pts[i].y = shape.pts[i].y * scale + rect.y;
        }

        sprintf(filePath, "%s/%s.pts", argv[2], fileName);
        write_pts_file(filePath, shape);
    }


    return 0;
}


