#include "tool.h"

int main(int argc, char **argv)
{
    if(argc < 4)
    {
        printf("Usage:%s [image list] [width] [out dir]\n", argv[1]);
        return 1;
    }

    std::vector<std::string> imageList;
    int size;
    int WIDTH;

    char outname[256], imgdir[128], imgname[128], ext[20];

    read_file_list(argv[1], imageList);

    size = imageList.size();

    WIDTH = atoi(argv[2]);

    if(WIDTH <= 10) return 1;

    for(int i = 0; i < size; i++)
    {
        cv::Mat img = cv::imread(imageList[i], 1);

        cv::resize(img, img, cv::Size(WIDTH, WIDTH * img.rows / img.cols));

        analysis_file_path(imageList[i].c_str(), imgdir, imgname, ext);
        sprintf(outname, "%s/%s.jpg", argv[3], imgname);

        cv::imwrite(outname, img);

        printf("%d\r", i);fflush(stdout);
    }
}
