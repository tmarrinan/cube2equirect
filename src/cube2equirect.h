#ifndef CUBE2EQUIRECT_H
#define CUBE2EQUIRECT_H
#include <string>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_image.h>
#include <jpeglib.h>
#include <png.h>

#ifdef __APPLE__
    #include <OpenGL/gl3.h>
#else
    #include <GL3/gl3.h>
#endif

#ifndef GL3_PROTOTYPES
#define GL3_PROTOTYPES 1
#endif

class cube2equirect {
private:
    SDL_Window *mainwindow;

    GLuint vertexArrayObject;
    GLuint vertexPositionBuffer;
    GLuint vertexTextureBuffer;
    GLuint vertexIndexBuffer;

    GLuint equirectFramebuffer;
    GLuint equirectTexture;

    GLuint cubeTextures[6];

    GLuint shaderProgram;
    GLint vertexPositionAttribute;
    GLint vertexTextureAttribute;

    GLint cubeLeftUniform;
    GLint cubeRightUniform;
    GLint cubeBottomUniform;
    GLint cubeTopUniform;
    GLint cubeBackUniform;
    GLint cubeFrontUniform;

    int frameCount;
    char frameIdx[7];
    std::string cubemapDir;
    std::string equirectDir;
    std::string imgExt;
    std::string outExt;
    std::string exePath;

    GLint equirectW;
    GLint equirectH;
    GLubyte *equirectPixels;

public:
    cube2equirect(SDL_Window *win, std::string exe);
    void initGL(std::string inDir, std::string outDir, int outRes, std::string outFmt);
    void render();
    bool hasMoreFrames();
    void initBuffers();
    void initRenderToTexture();
    void initCubeTextures();
    void updateCubeTextures();
    void initShaders(std::string name);
    GLint compileShader(std::string source, GLint type);
    void createShaderProgram(std::string name, GLint vertexShader, GLint fragmentShader);
    std::string readFile(std::string filename);
    void loadImage(std::string filename, GLuint texture, bool firstTime);
    bool saveImageJPEG(std::string filename, GLubyte *pixels, int width, int height);
    bool saveImagePNG(std::string filename, GLubyte *pixels, int width, int height);
};

#endif //CUBE2EQUIRECT_H
