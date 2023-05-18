#include "cube2equirect.h"
#include "imageio.hpp"

Cube2Equirect::Cube2Equirect(std::string in_dir, std::string out_dir, std::string out_format, int out_w, int out_h)
{
    _input_dir = makePath(in_dir);
    _output_dir = makePath(out_dir);
    _output_format = out_format;
    _output_width = out_w;
    _output_height = out_h;
    _output_pixels = new uint8_t[_output_width * _output_width * 4];

    _frame_count = 0;
    snprintf(_frame_idx, 7, "%06d", _frame_count);
    
    _vertex_position_attrib = 0;
    _vertex_texcoord_attrib = 1;
    
    init();
}

Cube2Equirect::~Cube2Equirect()
{
    delete[] _output_pixels;
}

// Public
bool Cube2Equirect::hasMoreFrames()
{
    bool more = false;
    
    std::string next_image = _input_dir + _frame_idx + "_left." + _input_format;
    struct stat info;
    if (stat(next_image.c_str(), &info) == 0 && !(info.st_mode & S_IFDIR)) {
        more = true;
    }

    return more;
}

void Cube2Equirect::renderNextFrame()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Update image textures
    updateTextureFromImage(_input_dir + _frame_idx + "_left." + _input_format, _cube_textures[0]);
    updateTextureFromImage(_input_dir + _frame_idx + "_right." + _input_format, _cube_textures[1]);
    updateTextureFromImage(_input_dir + _frame_idx + "_bottom." + _input_format, _cube_textures[2]);
    updateTextureFromImage(_input_dir + _frame_idx + "_top." + _input_format, _cube_textures[3]);
    updateTextureFromImage(_input_dir + _frame_idx + "_back." + _input_format, _cube_textures[4]);
    updateTextureFromImage(_input_dir + _frame_idx + "_front." + _input_format, _cube_textures[5]);
    
    // Render equirect image
    int i;
    GLint cube_uniforms[6];
    cube_uniforms[0] = _uniforms["cube_left"];
    cube_uniforms[1] = _uniforms["cube_right"];
    cube_uniforms[2] = _uniforms["cube_bottom"];
    cube_uniforms[3] = _uniforms["cube_top"];
    cube_uniforms[4] = _uniforms["cube_back"];
    cube_uniforms[5] = _uniforms["cube_front"];
    for (i = 0; i < 6; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, _cube_textures[i]);
        glUniform1i(cube_uniforms[i], i);
    }
    
    glBindVertexArray(_vertex_array);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    
    // Save pixel buffer as image
    glReadPixels(0, 0, _output_width, _output_height, GL_RGBA, GL_UNSIGNED_BYTE, _output_pixels);
    if (_output_format == "jpg")
    {
        iioWriteImageJpeg((_output_dir + "equirect_" + _frame_idx + ".jpg").c_str(), _output_width, _output_height, 4, 92, _output_pixels);
    }
    else
    {
        iioWriteImagePng((_output_dir + "equirect_" + _frame_idx + ".png").c_str(), _output_width, _output_height, 4, _output_pixels);
    }
    
    
    _frame_count++;
    snprintf(_frame_idx, 7, "%06d", _frame_count);
}

std::string Cube2Equirect::getEquirectImageFormat()
{
    return _output_format;
}

// Private
std::string Cube2Equirect::makePath(std::string path)
{
    if (path[path.length() - 1] != '/')
    {
        path += "/";
    }
    return path;
}

void Cube2Equirect::init()
{
    _program = glsl::createShaderProgram("shaders/cube2equirect.vert", "shaders/cube2equirect.frag");
    
    // Specify input and output attributes for the GPU program
    glBindAttribLocation(_program, _vertex_position_attrib, "vertex_position");
    glBindAttribLocation(_program, _vertex_texcoord_attrib, "vertex_texcoord");
    glBindFragDataLocation(_program, 0, "FragColor");

    // Link compiled GPU program
    glsl::linkShaderProgram(_program);

    // Get handles to uniform variables defined in the shaders
    glsl::getShaderProgramUniforms(_program, _uniforms);
    
    // Set background color
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    // Create fullscreen quad
    createVertexArrayObject();
    
    // Create cubemap textures
    createCubemapTextures();
    
    glUseProgram(_program);
}

void Cube2Equirect::createVertexArrayObject()
{
    glGenVertexArrays(1, &_vertex_array);
    glBindVertexArray(_vertex_array);
    
    // Vertices
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    GLfloat vertices[] = {
        -1.0, -1.0, -1.0,  // left,  bottom, back
        -1.0,  1.0, -1.0,  // left,  top,    back
         1.0, -1.0, -1.0,  // right, bottom, back
         1.0,  1.0, -1.0   // right, top,    back
    };
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(_vertex_position_attrib);
    glVertexAttribPointer(_vertex_position_attrib, 3, GL_FLOAT, false, 0, 0);
    
    // Texture Coordinates
    GLuint vertex_texcoord_buffer;
    glGenBuffers(1, &vertex_texcoord_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    GLfloat texcoords[] = {
        -1.0, -1.0,  // left,  bottom
        -1.0,  1.0,  // left,  top
         1.0, -1.0,  // right, bottom
         1.0,  1.0   // right, top
    };
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), texcoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(_vertex_texcoord_attrib);
    glVertexAttribPointer(_vertex_texcoord_attrib, 2, GL_FLOAT, false, 0, 0);
    
    // Faces of triangles
    GLuint vertex_index_buffer;
    glGenBuffers(1, &vertex_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
    GLushort vertex_indices[] = {
         0, 3, 1,
         3, 0, 2
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), vertex_indices, GL_STATIC_DRAW);
    
    // Unbind data
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Cube2Equirect::createCubemapTextures()
{
    int i;
    glGenTextures(6, _cube_textures);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    for (i = 0; i < 6; i++)
    {
        glBindTexture(GL_TEXTURE_2D, _cube_textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    struct stat info;
    if (stat((_input_dir + "000000_left.jpg").c_str(), &info) == 0 && !(info.st_mode & S_IFDIR))
    {
        _input_format = "jpg";
        if (_output_format != "jpg" && _output_format != "png") _output_format = "jpg";
    }
    else if (stat((_input_dir + "000000_left.png").c_str(), &info) == 0 && !(info.st_mode & S_IFDIR))
    {
        _input_format = "png";
        if (_output_format != "jpg" && _output_format != "png") _output_format = "png";
    }
    else
    {
        fprintf(stderr, "Cubemap images not found in directory '%s'\n", _input_dir.c_str());
        exit(EXIT_FAILURE);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Cube2Equirect::updateTextureFromImage(std::string filename, GLuint texture)
{
    int width, height;
    int channels = 4;
    uint8_t *pixels = iioReadImage(filename.c_str(), &width, &height, &channels);
    
    if (pixels == NULL)
    {
        fprintf(stderr, "Error: could not read image '%s'\n", filename.c_str());
        exit(EXIT_FAILURE);
    }
    
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    iioFreeImage(pixels);
}

/*
void cube2equirect::initGL(string inDir, string outDir, int outRes, string outFmt) {
    SDL_GL_SetSwapInterval(1);

    cubemapDir = inDir;
    if (cubemapDir[cubemapDir.length()-1] != '/')
        cubemapDir += "/";
    equirectDir = outDir;
    if (equirectDir[equirectDir.length()-1] != '/')
        equirectDir += "/";
    equirectW = outRes;
    equirectH = equirectW / 2;
    equirectPixels = (GLubyte*)malloc(3*equirectW*equirectH*sizeof(GLubyte));
    outExt = outFmt;

    glViewport(0, 0, equirectW, equirectH);

    initShaders("cube2equirect");
    initBuffers();
    initRenderToTexture();
    initCubeTextures();
}

void cube2equirect::render() {
    glBindFramebuffer(GL_FRAMEBUFFER, equirectFramebuffer);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // left side of cube
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cubeTextures[0]);
    glUniform1i(cubeLeftUniform, 0);

    // right side of cube
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cubeTextures[1]);
    glUniform1i(cubeRightUniform, 1);

    // bottom side of cube
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, cubeTextures[2]);
    glUniform1i(cubeBottomUniform, 2);

    // top side of cube
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, cubeTextures[3]);
    glUniform1i(cubeTopUniform, 3);

    // back side of cube
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, cubeTextures[4]);
    glUniform1i(cubeBackUniform, 4);

    // front side of cube
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, cubeTextures[5]);
    glUniform1i(cubeFrontUniform, 5);

    glBindVertexArray(vertexArrayObject);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);


    // read rendered image into pixel buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, equirectTexture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, equirectPixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    // save pixel buffer as image
    if (outExt == "jpg")
        saveImageJPEG(equirectDir + "equirect_" + frameIdx + ".jpg", equirectPixels, equirectW, equirectH);
    else
        saveImagePNG(equirectDir + "equirect_" + frameIdx + ".png", equirectPixels, equirectW, equirectH);


    frameCount++;
    sprintf(frameIdx, "%06d", frameCount);
    SDL_GL_SwapWindow(mainwindow);
}

bool cube2equirect::hasMoreFrames() {
    string nextImage = cubemapDir + frameIdx + "_left." + imgExt;

    struct stat info;
    if (stat(nextImage.c_str(), &info) == 0 && !(info.st_mode & S_IFDIR)) {
        return true;
    }
    return false;
}

void cube2equirect::initBuffers() {
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    // vertices
    glGenBuffers(1, &vertexPositionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPositionBuffer);
    GLfloat vertices[] = {
        -1.0, -1.0, -1.0,  // left,  bottom, back
        -1.0,  1.0, -1.0,  // left,  top,    back
         1.0, -1.0, -1.0,  // right, bottom, back
         1.0,  1.0, -1.0   // right, top,    back
    };
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertexPositionAttribute);
    glVertexAttribPointer(vertexPositionAttribute, 3, GL_FLOAT, false, 0, 0);

    // textures
    glGenBuffers(1, &vertexTextureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexTextureBuffer);
    GLfloat textureCoords[] = {
        -1.0, -1.0,  // left,  bottom
        -1.0,  1.0,  // left,  top
         1.0, -1.0,  // right, bottom
         1.0,  1.0   // right, top
    };
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), textureCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertexTextureAttribute);
    glVertexAttribPointer(vertexTextureAttribute, 2, GL_FLOAT, false, 0, 0);

    // faces of triangles
    glGenBuffers(1, &vertexIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
    GLushort vertexIndices[] = {
         0, 3, 1,
         3, 0, 2
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), vertexIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void cube2equirect::initRenderToTexture() {
    glGenFramebuffers(1, &equirectFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, equirectFramebuffer);

    glGenTextures(1, &equirectTexture);
    glBindTexture(GL_TEXTURE_2D, equirectTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, equirectW, equirectH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, equirectTexture, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cube2equirect::initCubeTextures() {
    glGenTextures(6, cubeTextures);

    struct stat info;
    if (stat((cubemapDir + "000000_left.jpg").c_str(), &info) == 0 && !(info.st_mode & S_IFDIR)) {
        imgExt = "jpg";
        if (outExt != "jpg" && outExt != "png") outExt = "jpg";
    }
    else if (stat((cubemapDir + "000000_left.png").c_str(), &info) == 0 && !(info.st_mode & S_IFDIR)) {
        imgExt = "png";
        if (outExt != "jpg" && outExt != "png") outExt = "png";
    }
    else {
        printf("cubemap images not found in directory \"%s\"\n", cubemapDir.c_str());
        SDL_Quit();
        exit(0);
    }

    loadImage(cubemapDir + "000000_left."   + imgExt, cubeTextures[0], true);
    loadImage(cubemapDir + "000000_right."  + imgExt, cubeTextures[1], true);
    loadImage(cubemapDir + "000000_bottom." + imgExt, cubeTextures[2], true);
    loadImage(cubemapDir + "000000_top."    + imgExt, cubeTextures[3], true);
    loadImage(cubemapDir + "000000_back."   + imgExt, cubeTextures[4], true);
    loadImage(cubemapDir + "000000_front."  + imgExt, cubeTextures[5], true);
}

void cube2equirect::updateCubeTextures() {
    loadImage(cubemapDir + frameIdx + "_left."   + imgExt, cubeTextures[0], false);
    loadImage(cubemapDir + frameIdx + "_right."  + imgExt, cubeTextures[1], false);
    loadImage(cubemapDir + frameIdx + "_bottom." + imgExt, cubeTextures[2], false);
    loadImage(cubemapDir + frameIdx + "_top."    + imgExt, cubeTextures[3], false);
    loadImage(cubemapDir + frameIdx + "_back."   + imgExt, cubeTextures[4], false);
    loadImage(cubemapDir + frameIdx + "_front."  + imgExt, cubeTextures[5], false);
}

void cube2equirect::initShaders(std::string name) {
    string vertSource = readFile(exePath + "shaders/" + name + ".vert");
    GLint vertexShader = compileShader(vertSource, GL_VERTEX_SHADER);

    string fragSource = readFile(exePath + "shaders/" + name + ".frag");
    GLint fragmentShader = compileShader(fragSource, GL_FRAGMENT_SHADER);
    
    createShaderProgram(name, vertexShader, fragmentShader);
}

GLint cube2equirect::compileShader(string source, GLint type) {
    GLint status;
    GLint shader = glCreateShader(type);

    const char *srcBytes = source.c_str();
    int srcLength = source.length();
    glShaderSource(shader, 1, &srcBytes, &srcLength);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        GLint length;
        char *info;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        info = SDL_stack_alloc(char, length+1);
        glGetShaderInfoLog(shader, length, NULL, info);
        fprintf(stderr, "Failed to compile shader:\n%s\n", info);
        SDL_stack_free(info);

        return -1;
    }
    else {
        return shader;
    }
}

void cube2equirect::createShaderProgram(string name, GLint vertexShader, GLint fragmentShader) {
    GLint status;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glBindAttribLocation(shaderProgram, 0, "aVertexPosition");
    glBindAttribLocation(shaderProgram, 1, "aVertexTextureCoord");
    glBindFragDataLocation(shaderProgram, 0, "FragColor");

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(status == 0) {
        fprintf(stderr, "Unable to initialize the shader program\n");
    }

    // set vertex array
    vertexPositionAttribute = glGetAttribLocation(shaderProgram, "aVertexPosition");
    // set texture coord array
    vertexTextureAttribute = glGetAttribLocation(shaderProgram, "aVertexTextureCoord");
    // set image textures
    cubeLeftUniform   = glGetUniformLocation(shaderProgram, "cubeLeftImage");
    cubeRightUniform  = glGetUniformLocation(shaderProgram, "cubeRightImage");
    cubeBottomUniform = glGetUniformLocation(shaderProgram, "cubeBottomImage");
    cubeTopUniform    = glGetUniformLocation(shaderProgram, "cubeTopImage");
    cubeBackUniform   = glGetUniformLocation(shaderProgram, "cubeBackImage");
    cubeFrontUniform  = glGetUniformLocation(shaderProgram, "cubeFrontImage");

    glUseProgram(shaderProgram);
}

string cube2equirect::readFile(string filename) {
    FILE *f = fopen(filename.c_str(), "rb");

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *text = (char*)malloc(fsize);
    fread(text, fsize, 1, f);
    fclose(f);

    return string(text, fsize);
}

void cube2equirect::loadImage(string filename, GLuint texture, bool firstTime) {
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (surface == NULL) {
        printf("Error: \"%s\"\n", SDL_GetError());
        return;
    }

    GLenum format = surface->format->BitsPerPixel == 32 ? GL_RGBA : GL_RGB;

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, texture);

    if (firstTime) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(surface);
}

bool cube2equirect::saveImageJPEG(string filename, GLubyte *pixels, int width, int height) {
    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        return false;
    }

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
     
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
     
    cinfo.image_width      = width;
    cinfo.image_height     = height;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality (&cinfo, 85, TRUE); // quality [0..100]
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer;
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer = (JSAMPROW)(pixels + cinfo.next_scanline * width * 3);
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }
    jpeg_finish_compress(&cinfo);
    
    fclose(fp);
    return true;
}

bool cube2equirect::saveImagePNG(string filename, GLubyte *pixels, int width, int height) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
        return false;

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, &info);
        return false;
    }

    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        png_destroy_write_struct(&png, &info);
        return false;
    }

    png_init_io(png, fp);
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_colorp palette = (png_colorp)png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
    if (!palette) {
        fclose(fp);
        png_destroy_write_struct(&png, &info);
        return false;
    }
    png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);
    png_write_info(png, info);
    png_set_packing(png);

    png_bytepp rows = (png_bytepp)png_malloc(png, height * sizeof(png_bytep));
    for (int i=0; i<height; i++) {
        rows[i] = (png_bytep)(pixels + i * width * 3);
    }

    png_write_image(png, rows);
    png_write_end(png, info);
    png_free(png, palette);
    png_destroy_write_struct(&png, &info);

    fclose(fp);
    png_free(png, rows);
    return true;
}
*/
