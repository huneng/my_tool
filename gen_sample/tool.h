#ifndef _OPENGL_TOOL_H_
#define _OPENGL_TOOL_H_

#include <stdlib.h>

#include <string>
#include <iostream>
#include <fstream>

#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>


#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#ifndef HU_PI
#define HU_PI 3.1415926
#endif

#ifndef HU_FREE
#define HU_FREE(arr) \
{ \
    if(arr != NULL) \
        free(arr); \
    arr = NULL; \
}
#endif

#ifndef HU_MIN
#define HU_MIN(a, b) ((a) < (b) ? (a) : (b))
#define HU_MAX(a, b) ((a) > (b) ? (a) : (b))
#define HU_SWAP(x, y, type) {type tmp = (x); (x) = (y); (y) = (tmp);}
#endif


int read_file_list(const char *filePath, std::vector<std::string> &fileList);
void analysis_file_path(const char* filePath, char *rootDir, char *fileName, char *ext);

GLFWwindow* glfw_init_and_create_window(int width, int height, int mode, int value);

GLuint compile_shader(const char* filePath, GLenum shaderType, char **log);
GLuint link_program(GLuint* shaderIDs, int shaderSize, char **log);
GLuint load_shaders(const char *vertexShaderFile, const char *fragmentShaderFile);


GLuint create_texture_using_image(uint8_t *image, int width, int height);

#endif
