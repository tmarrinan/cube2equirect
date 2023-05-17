#ifndef CUBE2EQUIRECT_H
#define CUBE2EQUIRECT_H

#include <string>
#include <map>
#include <sys/stat.h>
#include "glslloader.h"

class Cube2Equirect {
private:
    std::string _input_dir;
    std::string _input_format;
    std::string _output_dir;
    std::string _output_format;
    int _output_width;
    int _output_height;
    uint8_t *_output_pixels;
    int _frame_count;
    char _frame_idx[7];
    GLuint _program;
    GLint _vertex_position_attrib;
    GLint _vertex_texcoord_attrib;
    std::map<std::string,GLint> _uniforms;
    GLuint _vertex_array;
    GLuint _cube_textures[6];
    
    std::string makePath(std::string path);
    void init();
    void createVertexArrayObject();
    void createCubemapTextures();
    void updateTextureFromImage(std::string filename, GLuint texture);

    /*
    GLuint vertex_array;
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
    */

public:
    Cube2Equirect(std::string in_dir, std::string out_dir, std::string out_format, int out_w, int out_h);
    ~Cube2Equirect();
    
    bool hasMoreFrames();
    void renderNextFrame();

    /*
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
    */
};

#endif //CUBE2EQUIRECT_H

