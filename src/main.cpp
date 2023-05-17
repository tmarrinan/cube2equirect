#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
/*#ifdef __linux__
#define MESA_EGL_NO_X11_HEADERS
#endif
#include "glad/glad_egl.h"
#include <EGL/egl.h>
#include <GL/gl.h>*/
#include "glad/egl.h"
#include "glad/gl.h"

#include "cube2equirect.h"


typedef struct AppData {
    std::string cube_data_dir;      // input image data directory
    std::string equirect_data_dir;  // output image/video data directory
    int width;                      // output image/video width
    int height;                     // output image/video height
    std::string out_format;         // output file format
    int video_framerate;            // output video frame rate
    EGLDisplay egl_display;         // EGL display
    EGLSurface egl_surface;         // EGL surface
} AppData;


void parseArguments(int argc, char **argv, AppData *app_ptr);

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("\n");
        printf("  Usage: cube2equirect [options]\n");
        printf("\n");
        printf("  Options:\n");
        printf("\n");
        printf("    -i, --input <DIRECTORY>      directory with cubemap image set sequence\n");
        printf("    -o, --output <DIRECTORY>     directory to save equirectangular images [Default: \'output/\']\n");
        printf("    -h, --h-resolution <NUMBER>  horizontal resolution of output images [Default: 3840]\n");
        printf("    -f, --format <IMG_FORMAT>    output image format (\'jpg\', \'png\', or \'mp4\') [Default: same as input]\n");
        printf("    -r, --framerate <NUMBER>     number of images per second (for video output) [Default: 24]\n");
        printf("\n");
        return 0;
    }

    AppData app;
    parseArguments(argc, argv, &app);

    struct stat info;
    if (stat(app.cube_data_dir.c_str(), &info) != 0) {
        fprintf(stderr, "\"%s\" does not exist or cannot be accessed, please specify directory with cubemap images\n", app.cube_data_dir.c_str());
        return EXIT_FAILURE;
    }
    else if (!(info.st_mode & S_IFDIR)) {
        fprintf(stderr, "\"%s\" is not a directory, please specify directory with cubemap images\n", app.cube_data_dir.c_str());
        return EXIT_FAILURE;
    }
    
    // Prepare for EGL initialization
    int egl_version = gladLoaderLoadEGL(NULL);
    if (!egl_version)
    {
        fprintf(stderr, "Error: could not pre-initialize GLAD EGL\n");
        return EXIT_FAILURE;
    }
    
    // Initialize EGL
    app.egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint egl_major, egl_minor;
    eglInitialize(app.egl_display, &egl_major, &egl_minor);
    egl_version = gladLoaderLoadEGL(app.egl_display);
    if (!egl_version)
    {
        fprintf(stderr, "Error: could not initialize EGL display\n");
        return EXIT_FAILURE;
    }

    // Initialize GL attributes
    EGLint num_configs;
    EGLConfig egl_config;
    static const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    eglChooseConfig(app.egl_display, config_attribs, &egl_config, 1, &num_configs);

    // Create EGL surface
    static const EGLint pbuffer_attribs[] = {
        EGL_WIDTH, app.width,
        EGL_HEIGHT, app.height,
        EGL_NONE
    };
    app.egl_surface = eglCreatePbufferSurface(app.egl_display, egl_config, pbuffer_attribs);

    // Bind API
    eglBindAPI(EGL_OPENGL_API);

    // Create OpenGL context and make it current
    static const EGLint context_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_NONE
    };
    EGLContext egl_ctx = eglCreateContext(app.egl_display, egl_config, EGL_NO_CONTEXT, context_attribs);
    eglMakeCurrent(app.egl_display, app.egl_surface, app.egl_surface, egl_ctx);
    
    // Initialize GLAD (OpenGL Extenstions)
    int ogl_version = gladLoaderLoadGL();
    if (!ogl_version)
    {
        fprintf(stderr, "Error: could not initialize GLAD OpenGL extensions\n");
        return EXIT_FAILURE;
    }
    

    const unsigned char* gl_version = glGetString(GL_VERSION);
    const unsigned char* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("Using OpenGL %s, GLSL %s\n", gl_version, glsl_version);

    // Convert cube maps to equirectangular images    
    Cube2Equirect *converter = new Cube2Equirect(app.cube_data_dir, app.equirect_data_dir, app.out_format, app.width, app.height);
    while (converter->hasMoreFrames()) {
        converter->renderNextFrame();
        eglSwapBuffers(app.egl_display, app.egl_surface);
    }
    printf("glError: %d\n", glGetError());

    // Clean up
    delete converter;
    gladLoaderUnloadGL();
    eglDestroyContext(app.egl_display, egl_ctx);
    eglDestroySurface(app.egl_display, app.egl_surface);
    eglTerminate(app.egl_display);
    gladLoaderUnloadEGL();


    return EXIT_SUCCESS;
}

void parseArguments(int argc, char **argv, AppData *app_ptr) {
    app_ptr->equirect_data_dir = "output/";
    app_ptr->width = 3840;
    app_ptr->height = app_ptr->width / 2;
    app_ptr->out_format = "";
    app_ptr->video_framerate = 24;
    bool has_input = false;

    int arg_idx = 1;
    while (argc > (arg_idx + 1))
    {
        if (strcmp(argv[arg_idx], "-i") == 0 || strcmp(argv[arg_idx], "--input") == 0)
        {
            app_ptr->cube_data_dir = argv[arg_idx + 1];    
            has_input = true;
        }
        else if (strcmp(argv[arg_idx], "-o") == 0 || strcmp(argv[arg_idx], "--output") == 0)
        {
            app_ptr->equirect_data_dir = argv[arg_idx + 1];
        }
        else if (strcmp(argv[arg_idx], "-h") == 0 || strcmp(argv[arg_idx], "--h-resolution") == 0)
        {
            int res = atoi(argv[arg_idx + 1]);
            if (res > 1)
            {
                app_ptr->width = res;
                app_ptr->height = res / 2;
            }
        }
        else if (strcmp(argv[arg_idx], "-f") == 0 || strcmp(argv[arg_idx], "--format") == 0)
        {
            app_ptr->out_format = argv[arg_idx + 1];
        }
        else if (strcmp(argv[arg_idx], "-r") == 0 || strcmp(argv[arg_idx], "--framerate") == 0)
        {
            int fr = atoi(argv[arg_idx + 1]);
            if (fr > 0)
            {
                app_ptr->video_framerate = fr;
            }
        }
        arg_idx += 2;
    }

    if (!has_input) {
        fprintf(stderr, "please specify an input directory with cubemap images\n");
        exit(EXIT_FAILURE);
    }
}

