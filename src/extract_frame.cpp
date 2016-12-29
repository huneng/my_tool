#include "tool.h"


int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [video] [outdir]\n", argv[0]);
        return 1;
    }

    char rootDir[128], fname[128], ext[20], outname[256];
    analysis_file_path(argv[1], rootDir, fname, ext);

    cv::VideoCapture cap(argv[1]);

    if(!cap.isOpened()){
        printf("Can't open video %s\n", argv[1]);
        return 1;
    }

    long totalFrameNumber = cap.get(CV_CAP_PROP_FRAME_COUNT);
    cv::Mat frame;

    for(long frameNumber = 0; frameNumber < totalFrameNumber; frameNumber++){
        cap >> frame;

        if(frame.empty())
            continue;

        sprintf(outname, "%s/%06ld.jpg", argv[2], frameNumber + 1);

        if(!cv::imwrite(outname, frame)){
            printf("Can't write frame %s\n", outname);
            break;
        }

        printf("%.2f%%\r", 100.0 * frameNumber / totalFrameNumber);
        fflush(stdout);
    }

    return 0;
}

