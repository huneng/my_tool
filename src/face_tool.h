#ifndef _FACE_TOOL_H_
#define _FACE_TOOL_H_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>

#include "base.h"

typedef struct {
    cv::Mat img;
    Shape shape;
    std::string imgName;
} Sample;


void normalize_sample(cv::Mat &src, cv::Mat &patch, int winSize, float factor, Shape &shape);
int save_sample(cv::Mat &img, Shape &shape, const char *outDir, const char *outName);
void load_sample(const char *imgPath, cv::Mat &img, Shape &shape);

void show_shape(Shape &shape, const char *winName);
void show_shape(cv::Mat &img, Shape &shape, const char *winName);

void extract_area_from_image(cv::Mat &img, cv::Mat &patch, cv::Rect &rect);
#endif
