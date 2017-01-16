#ifndef _FACE_TOOL_H_
#define _FACE_TOOL_H_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>

#include "def_types.h"

typedef std::vector<cv::Point2f> Shape;


typedef struct{
    float sina;
    float cosa;
    float scale;

    cv::Point2f cen1, cen2;
} TranArgs;


typedef struct {
    cv::Mat img;
    Shape shape;
    std::string imgName;
} Sample;


int read_pts_file(const char *filePath, std::vector<cv::Point2f> &shapes);
int write_pts_file(const char *filePath, std::vector<cv::Point2f> &shapes);

void get_shape_rect(std::vector<cv::Point2f> &shape, cv::Rect &rect);
int calc_face_rect(int width, int height, std::vector<cv::Point2f> &shape, cv::Rect &rect);
void load_sample(const char *imgPath, cv::Mat &img, Shape &shape);
void normalize_sample(cv::Mat &src, cv::Mat &patch, int winSize, std::vector<cv::Point2f> &shape);
int write_sample(cv::Mat &img, std::vector<cv::Point2f> &shape, const char *outDir, const char *outName);

void mirror_points(std::vector<cv::Point2f> &shape);
void draw_points(cv::Mat &img, std::vector<cv::Point2f> &pts, cv::Scalar color);

void similarity_transform(Shape &src, Shape &dst, TranArgs &arg);

void show_shape(Shape &shape, const char *winName);
void show_shape(cv::Mat &img, Shape &shape, const char *winName);
void show_shape(uint8_t *img, int width, int height, int stride, Shape &shape, const char *winName);

int read_samples(const char *listFile, std::vector<Sample*> &samples, int winSize);
void release(std::vector<Sample*> &samples);

void calc_mean_shape_global(std::vector<Shape > &shapes, int SHAPE_SIZE, Shape &meanShape);

void affine_sample(cv::Mat& src, Shape &shape, float angle, float scale, cv::Point2f &center);

void transform_sample(cv::Mat &img, Shape &shape);

#endif
