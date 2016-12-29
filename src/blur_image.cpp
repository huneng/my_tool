#include "tool.h"

#include <math.h>
#include <stdlib.h>

#include <inttypes.h>


int main(int argc, char **argv)
{
    if(argc<3)
    {
        printf("Usage:%s [image list] [output dir] \n", argv[0]);
        return 1;
    }

    std::vector<std::string> imageList;

    cv::Mat img, dst;

    char *outdir, outname[128], imgname[40], imgdir[128], ext[20];
    int len, size;

    outdir = argv[2];
    len = strlen(outdir);

    if(outdir[len-1] == '/')
        outdir[len-1] = '\0';


    read_file_list(argv[1], imageList);

    size = imageList.size();

    for(int i = 0; i < size; i++)
    {
        img = cv::imread(imageList[i], 1);

        analysis_file_path(imageList[i].c_str(), imgdir, imgname, ext);

        cv::GaussianBlur(img, img, cv::Size(7, 7), 2.0);

        sprintf(outname, "%s/%s.jpg", outdir, imgname);

        if(!cv::imwrite(outname, img))
        {
            printf("Write image error %s\n", outname);
        }

        printf("%d\r", i);
        fflush(stdout);
    }

    return 0;
}



