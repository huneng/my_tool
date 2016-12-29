#include "tool.h"


int main(int argc, char **argv)
{
    if(argc < 3)
    {
        printf("Usage:%s [image list] [out dir]\n", argv[1]);
        return 1;
    }

    std::vector<std::string> imageList;
    int size;

    char outname[256], imgdir[128], imgname[128], ext[20];

    read_file_list(argv[1], imageList);

    size = imageList.size();

    for(int i = 0; i < size; i++)
    {
        cv::Mat img = cv::imread(imageList[i], 0);

        analysis_file_path(imageList[i].c_str(), imgdir, imgname, ext);

        sprintf(outname, "%s/%s.bmp", argv[2], imgname);

        cv::imwrite(outname, img);
    }
}
