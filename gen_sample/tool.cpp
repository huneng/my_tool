#include "tool.h"
#include <string.h>


int read_file_list(const char *filePath, std::vector<std::string> &fileList)
{
    char line[512];
    FILE *fin = fopen(filePath, "r");

    if(fin == NULL){
        printf("Can't open file: %s\n", filePath);
        return -1;
    }

    while(fscanf(fin, "%s\n", line) != EOF){
        fileList.push_back(line);
    }

    fclose(fin);

    return 0;
}


void analysis_file_path(const char* filePath, char *rootDir, char *fileName, char *ext)
{
    int len = strlen(filePath);
    int idx = len - 1, idx2 = 0;

    while(idx >= 0){
        if(filePath[idx] == '.')
            break;
        idx--;
    }

    if(idx >= 0){
        strcpy(ext, filePath + idx + 1);
        ext[len - idx] = '\0';
    }
    else {
        ext[0] = '\0';
        idx = len - 1;
    }

    idx2 = idx;
    while(idx2 >= 0){
#if defined(WIN32)
        if(filePath[idx2] == '\\')
#elif defined(linux)
        if(filePath[idx2] == '/')
#endif
            break;
        idx2 --;
    }

    if(idx2 > 0){
        strncpy(rootDir, filePath, idx2);
        rootDir[idx2] = '\0';
    }
    else{
        rootDir[0] = '.';
        rootDir[1] = '\0';
    }

    strncpy(fileName, filePath + idx2 + 1, idx - idx2 - 1);
    fileName[idx - idx2 - 1] = '\0';
}


GLFWwindow* glfw_init_and_create_window(int width, int height, int mode, int value){
    GLFWwindow *window = NULL;

    // Initialise GLFW
    if( !glfwInit() ){
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return NULL;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( width, height, "window", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwTerminate();
        return NULL;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, mode, value);

    return window;
}


GLuint compile_shader(const char* filePath, GLenum shaderType, char **log){
    GLuint shaderID;

    std::string fileContent, line;
    std::ifstream fin(filePath, std::ios::in);
    const char *ptr = NULL;
    GLint status;

    if(!fin.is_open()){
        printf("Can't open file %s\n", filePath);
        return 0;
    }

    line = "";

    while(std::getline(fin, line))
        fileContent += "\n" + line;

    fin.close();

    shaderID = glCreateShader(shaderType);

    ptr = fileContent.c_str();

    glShaderSource(shaderID, 1, &ptr, NULL);
    glCompileShader(shaderID);

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if(!status){
        GLint length;

        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
        *log = (char*)malloc(sizeof(char) * length);

        glGetShaderInfoLog(shaderID, length, NULL, *log);
        glDeleteShader(shaderID);

        printf("Load shader error: %s\n", *log);
        return 0;
    }

    return shaderID;
}


GLuint link_program(GLuint* shaderIDs, int shaderSize, char **log){
    GLint status;
    GLuint programID = glCreateProgram();

    for(int i = 0; i < shaderSize; i++)
        glAttachShader(programID, shaderIDs[i]);

    glLinkProgram(programID);

    for(int i = 0; i < shaderSize; i++)
        glDetachShader(programID, shaderIDs[i]);

    glGetProgramiv(programID, GL_LINK_STATUS, &status);
    if(!status){
        GLint length;

        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);
        *log = (char*)malloc(sizeof(char) * length);
        glGetProgramInfoLog(programID, length, NULL, *log);
        printf("%s\n", *log);

        glDeleteProgram(programID);
        return 0;
    }

    return programID;
}


GLuint load_shaders(const char *vertexShaderFile, const char *fragmentShaderFile){
    GLuint shaderIDs[2];
    GLuint programID;
    char *log = NULL;

    shaderIDs[0] = compile_shader(vertexShaderFile, GL_VERTEX_SHADER, &log); HU_FREE(log);
    if(shaderIDs[0] == 0)
        return 0;

    shaderIDs[1] = compile_shader(fragmentShaderFile, GL_FRAGMENT_SHADER, &log); HU_FREE(log);
    if(shaderIDs[1] == 0)
        return 0;

    programID = link_program(shaderIDs, 2, &log); HU_FREE(log);

    glDeleteShader(shaderIDs[0]);
    glDeleteShader(shaderIDs[1]);

    return programID;
}


GLuint create_texture_using_image(uint8_t *image, int width, int height){
    GLuint textureID;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}
