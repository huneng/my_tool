#include "tool.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define TRAINS_FORM_SIZE 10000

typedef std::vector<cv::Point2f> Shape;
#define PTS_SIZE 25

int read_pts_file(const char *filePath, Shape &shapes)
{
    FILE *fin = fopen(filePath, "r");

    char line[256], *ptr;
    cv::Point2f shape;

    if(fin == NULL){
        printf("Can't open file %s\n", filePath);
        return 0;
    }

    ptr = fgets(line, 255, fin);
    ptr = fgets(line, 255, fin);
    ptr = fgets(line, 255, fin);

    shapes.clear();

    while(fgets(line, 255, fin) != NULL){
        int ret = sscanf(line, "%f %f", &shape.x, &shape.y);
        if(ret == 0) break;
        shapes.push_back(shape);
    }

    fclose(fin);

    return shapes.size();
}


int write_pts_file(const char *filePath, Shape &shapes)
{
    FILE *fout = fopen(filePath, "w");

    char line[256];
    cv::Point2f shape;

    if(fout == NULL){
        printf("Can't open file %s\n", filePath);
        return 1;
    }

    fprintf(fout, "version: 1\n");
    fprintf(fout, "n_points:  %d\n", PTS_SIZE);
    fprintf(fout, "{\n");

    int size = shapes.size();

    for(int i = 0; i < size; i++){
        fprintf(fout, "%f %f\n", shapes[i].x, shapes[i].y);
    }

    fprintf(fout, "}");
    fclose(fout);

    return 0;
}


void get_face_rect(Shape &shape, cv::Rect &rect, float factor){
    int ptsSize = shape.size();

    float minx = FLT_MAX, maxx = -FLT_MAX;
    float miny = FLT_MAX, maxy = -FLT_MAX;

    for(int i = 0; i < ptsSize; i++){
        minx = HU_MIN(minx, shape[i].x);
        maxx = HU_MAX(maxx, shape[i].x);

        miny = HU_MIN(miny, shape[i].y);
        maxy = HU_MAX(maxy, shape[i].y);
    }

    float cx = (maxx + minx) / 2;
    float cy = (maxy + miny) / 2;

    float w = (maxx - minx + 1);
    float h = (maxy - miny + 1);

    float faceSize = HU_MAX(w, h) * factor;

    rect.x = cx - faceSize / 2;
    rect.y = cy - faceSize / 2;
    rect.width = faceSize;
    rect.height = faceSize;
}


void extract_face_from_image(cv::Mat &src, cv::Rect &rect, cv::Mat &patch, Shape &shape){
    int x0 = rect.x;
    int y0 = rect.y;
    int x1 = rect.x + rect.width  - 1;
    int y1 = rect.y + rect.height - 1;

    int width  = src.cols;
    int height = src.rows;
    int w, h;

    int ptsSize;

    int bl = 0, bt = 0, br = 0, bb = 0;

    cv::RNG rng(cv::getTickCount());

    if(x0 < 0) {
        bl = -x0;
        x0 = 0;
    }

    if(y0 < 0){
        bt = -y0;
        y0 = 0;
    }

    if(x1 > width - 1){
        br = x1 - width + 1;
        x1 = width - 1;
    }

    if(y1 > height - 1){
        bb = y1 - height + 1;
        y1 = height - 1;
    }


    w = rect.width - bl - br;
    h = rect.height - bt - bb;

    patch = cv::Mat(rect.height, rect.width, src.type(), cv::Scalar::all(0));
    patch(cv::Rect(bl, bt, w, h)) += src(cv::Rect(x0, y0, w, h));

    ptsSize = shape.size();

    for(int i = 0; i < ptsSize; i++){
        shape[i].x += (bl - x0);
        shape[i].y += (bt - y0);
    }
}


float bgVtxData[] = {
    -1.0,  1.0,  0.0,
    -1.0, -1.0,  0.0,
     1.0, -1.0,  0.0,
     1.0,  1.0,  0.0,
};


float bgTexData[] = {
    0.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
    1.0, 0.0,
};



GLuint bgIndices[] = {0, 1, 2, 0, 3, 2};

void rotate_image_180(uint8_t *img, int width, int height, int stride);
void set_model_transform_matrix(glm::mat4 &model, glm::vec3 svec, glm::vec3 tvec, float ex, float ey, float ez);

void show_shape(cv::Mat &img, Shape &shape, const char *winName);

void transform_image(cv::Mat &img);

#define WINW 640
#define WINH 480

int main(int argc, char **argv){
    if(argc < 3){
        printf("Usage: %s [image list] [out dir]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imgList;
    int size;

    read_file_list(argv[1], imgList);
    size = imgList.size();

    GLFWwindow *window = NULL;

    GLuint programID;
    GLuint texID;

    GLuint mmatID, vmatID, pmatID;
    GLuint VAO, VBO, TBO, IBO;

    GLuint texShaderID;


    window = glfw_init_and_create_window(WINW, WINH, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    programID = load_shaders("image_shader.vs", "image_shader.fg");

    mmatID = glGetUniformLocation(programID, "model");
    vmatID = glGetUniformLocation(programID, "view");
    pmatID = glGetUniformLocation(programID, "projection");
    texShaderID = glGetUniformLocation(programID, "texDiff1");

    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &TBO);
    glGenBuffers(1, &IBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVtxData), bgVtxData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)NULL);

    glBindBuffer(GL_ARRAY_BUFFER, TBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgTexData), bgTexData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)NULL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bgIndices), bgIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    glm::mat4 pmat(1.0), mmat(1.0), vmat(1.0);

    glViewport(0, 0, WINW, WINH);
    glEnable(GL_DEPTH_TEST);

    uint8_t *imgData = new uint8_t[WINW * WINH * 3];

    for(int i = 0; i < size; i++){
        cv::Mat img, patch;
        Shape shape;
        char rootDir[128], fileName[128], ext[30], filePath[256], prefix[256];
        const char *imgPath = imgList[i].c_str();
        cv::Rect rect;

        img = cv::imread(imgPath, 1);
        if(img.empty())
            break;

        analysis_file_path(imgPath, rootDir, fileName, ext);
        sprintf(filePath, "%s/%s.pts", rootDir, fileName);

        if(read_pts_file(filePath, shape) == 0)
            break;

        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

        sprintf(filePath, "%s/%s", argv[2], fileName);

        get_face_rect(shape, rect, 3);

        int cx = rect.x + rect.width / 2;
        int cy = rect.y + rect.height / 2;

        int faceSize = rect.width;

        rect.width = ((faceSize + 3) >> 2) << 2;
        rect.height = (rect.width >> 2) * 3;

        rect.x = cx - rect.width / 2;
        rect.y = cy - rect.height / 2;

        extract_face_from_image(img, rect, patch, shape);

        float scale = (float(WINW)) / patch.cols;

        cv::resize(patch, patch, cv::Size(WINW, WINH));
        for(int p = 0; p < PTS_SIZE; p++){
            shape[p] *= scale;
        }

        sprintf(prefix, "%s/%s", argv[2], fileName);

        {
            glm::vec4 srcPts[PTS_SIZE];
            cv::RNG rng(cv::getTickCount());
            char outPath[256];
            float std = 0.5;
            int w, h;

            cx = WINW >> 1;
            cy = WINH >> 1;
            w = WINW >> 1;
            h = WINH >> 1;

            for(int p = 0; p < PTS_SIZE; p++)
                srcPts[p] = glm::vec4((shape[p].x - cx) / w, (cy - shape[p].y) / h, 0, 1.0);

            texID = create_texture_using_image(patch.data, patch.cols, patch.rows);

            for(int frame = 0; frame < TRAINS_FORM_SIZE; frame ++){
                float ex = rng.uniform(-HU_PI / 9, HU_PI / 9);
                float ey = rng.uniform(-HU_PI / 9, HU_PI / 9);
                float ez = rng.uniform(-HU_PI / 9, HU_PI / 9);

                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                set_model_transform_matrix(mmat, glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.0, 0.0, 0.0), ex, ey, ez);

                glUseProgram(programID);

                glUniformMatrix4fv(pmatID, 1, GL_FALSE, glm::value_ptr(pmat));
                glUniformMatrix4fv(mmatID, 1, GL_FALSE, glm::value_ptr(mmat));
                glUniformMatrix4fv(vmatID, 1, GL_FALSE, glm::value_ptr(vmat));

                glActiveTexture(GL_TEXTURE0);
                glUniform1i(texShaderID, 0);
                glBindTexture(GL_TEXTURE_2D, texID);

                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);

                glfwSwapBuffers(window);
                glfwPollEvents();

                GLint viewPort[4];
                glGetIntegerv(GL_VIEWPORT, viewPort);

                glReadPixels(viewPort[0], viewPort[1], viewPort[2], viewPort[3], GL_BGR, GL_UNSIGNED_BYTE, imgData);

                for(int p = 0; p < PTS_SIZE; p++){
                    glm::vec4 vec = pmat * vmat * mmat * srcPts[p];
                    shape[p].x = vec[0] * w + cx;
                    shape[p].y = cy - vec[1] * h;
                }

                rotate_image_180(imgData, WINW, WINH, WINW * 3);

                cv::Mat sImg(WINH, WINW, CV_8UC3, imgData);

                transform_image(sImg);

                sprintf(outPath, "%s_%d.jpg", prefix, frame);
                cv::imwrite(outPath, sImg);

                sprintf(outPath, "%s_%d.pts", prefix, frame);

                write_pts_file(outPath, shape);
            }
        }

        printf("%d\r", i); fflush(stdout);
    }

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &TBO);
    glDeleteBuffers(1, &IBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(1, &texID);

    glDeleteProgram(programID);
    glfwDestroyWindow(window);

    return 0;
}


void set_model_transform_matrix(glm::mat4 &model, glm::vec3 svec, glm::vec3 tvec, float ex, float ey, float ez){
    model = glm::mat4(1.0);

    model = glm::rotate(model, ex, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, ey, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, ez, glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::translate(model, tvec);
    model = glm::scale(model, svec);
}


void rotate_image_180(uint8_t *img, int width, int height, int stride){
    int cx = width >> 1;
    int cy = height >> 1;

    for(int y = 0; y < height; y++){
        for(int x = 0; x < cx; x++){
            int x0 = x * 3;
            int x1 = (width - 1 - x) * 3;

            HU_SWAP(img[x0], img[x1], uint8_t);
            HU_SWAP(img[x0 + 1], img[x1 + 1], uint8_t);
            HU_SWAP(img[x0 + 2], img[x1 + 2], uint8_t);
        }
    }

    uint8_t *buffer = new uint8_t[stride];
    for(int y = 0; y < cy; y++){
        int y0 = y;
        int y1 = height - 1 - y0;

        memcpy(buffer, img + y0 * stride, sizeof(uint8_t) * stride);
        memcpy(img + y0 * stride, img + y1 * stride, sizeof(uint8_t) * stride);
        memcpy(img + y1 * stride, buffer, sizeof(uint8_t) * stride);
    }

    delete [] buffer;
}


void show_shape(cv::Mat &img, Shape &shape, const char *winName){
    cv::Mat cImg;
    int ptsSize;

    if(img.channels() == 3)
        img.copyTo(cImg);
    else
        cv::cvtColor(img, cImg, cv::COLOR_GRAY2BGR);

    ptsSize = shape.size();

    for(int i = 0; i < ptsSize; i++){
        cv::circle(cImg, shape[i], 3, cv::Scalar(0, 255, 0), -1);
    }

    cv::imshow(winName, cImg);
    cv::waitKey(1);
}



void transform_image(cv::Mat &img){
    static cv::RNG rng(cv::getTickCount());

    int cols = img.cols;
    int rows = img.rows;

    std::vector<cv::Mat> imgs;

    cv::split(img, imgs);

    for(int i = 0; i < 3; i++){
        if(rng.uniform(0, 8) == 0){
            int ksize = rng.uniform(1, 3);
            ksize = ksize * 2 + 1;

            cv::GaussianBlur(imgs[i], imgs[i], cv::Size(ksize, ksize), 0, 0);
        }

        if(rng.uniform(0, 8) == 1){
            uint8_t *data = imgs[i].data;
            for(int y = 0; y < rows; y++){
                for(int x = 0; x < cols; x++){
                    int noise = rng.uniform(-16, 16);

                    if(data[x] > 16 && data[x] < 240)
                        data[x] += noise;
                }

                data += imgs[i].step;
            }
        }

        if(rng.uniform(0, 8) == 1){
            float sum = 0;

            uint8_t *data = imgs[i].data;

            for(int y = 0; y < rows; y++){
                for(int x = 0; x < cols; x++)
                    sum += data[x];

                data += imgs[i].step;
            }

            sum /= (rows * cols);
            sum ++;

            data = imgs[i].data;
            for(int y = 0; y < rows; y++){
                for(int x = 0; x < cols; x++)
                    data[x] = ((data[x] - sum) / (data[x] + sum) + 1) / 2.0 * 255;

                data += imgs[i].step;
            }
        }

        if(rng.uniform(0, 8) == 2)
            cv::equalizeHist(imgs[i], imgs[i]);
    }

    cv::merge(imgs, img);
}
