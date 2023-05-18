#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
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
void convertImageSequenceToVideo(std::string image_dir, std::string img_format, int image_framerate);

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
    
    printf("-----------------\n| Cube2Equirect |\n-----------------\n");

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
    
    // Compile image sequence to video (if desired)
    if (app.out_format == "mp4")
    {
        convertImageSequenceToVideo(app.equirect_data_dir, converter->getEquirectImageFormat(), app.video_framerate);
    }

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

void convertImageSequenceToVideo(std::string image_dir, std::string img_format, int image_framerate)
{
    char *ffmpeg_cmd = new char[512];
    int framerate_mult = (24 % image_framerate == 0) ? (24 / image_framerate) : (24 / image_framerate) + 1;
    int video_framerate = (image_framerate < 24) ? framerate_mult * image_framerate : image_framerate;
#ifdef _WIN32
    const char *o_null = "NUL";
#else
    const char *o_null = "/dev/null";
#endif
    snprintf(ffmpeg_cmd, 512, "ffmpeg -y -start_number 0 -r %d -i \"%sequirect_%%06d.%s\" -r %d -c:v libx264 -an -pix_fmt yuv420p \"%sequirect.mp4\" > %s 2>&1", image_framerate, image_dir.c_str(), img_format.c_str(), video_framerate, image_dir.c_str(), o_null);
    
    int err = system(ffmpeg_cmd);
    if (err != 0)
    {
        fprintf(stderr, "Warning: ffmpeg conversion to mp4 not successful. Saved as image sequence instead.\n");
    }
    else
    {
        char *img_filename = new char[256];
        int i = 0;
        do
        {
            snprintf(img_filename, 256, "%sequirect_%06d.%s", image_dir.c_str(), i, img_format.c_str());
            i++;
        } while (remove(img_filename) == 0);
        
        delete[] img_filename;
    }
    delete[] ffmpeg_cmd;
}

