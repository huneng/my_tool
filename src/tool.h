#ifndef _TOOL_H_
#define _TOOL_H_


#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>

#include "def_types.h"

int read_file_list(const char *filePath, std::vector<std::string> &fileList);
void analysis_file_path(const char* filePath, char *rootDir, char *fileName, char *ext);

void npd_normalize(uint8_t *img, int width, int height, int stride);
void noise_image(uint8_t *img, int width, int height, int stride);
void inverse_color(uint8_t *img, int width, int height, int stride);
void mirror_image(uint8_t *img, int width, int height, int stride);

cv::Point2f calculate_central(std::vector<cv::Point2f> &shape);

#endif
