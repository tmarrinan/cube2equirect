#include <iostream>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <SDL.h>

#include "cube2equirect.h"

#define PROGRAM_NAME "Cube2Equirect"

using namespace std;


SDL_Window *mainwindow;         // Window handle
SDL_GLContext maincontext;      // OpenGL context handle
cube2equirect *renderer;        // Renderer
string cubeDataDir;             // Input data directory
string equirectDataDir;         // Output data directory
int hResolution;                // Output horizontal resolution
string outFormat;               // Output file format
int videoFrameRate;             // Output video images per second
string exePath;                 // Executable path

void parseArguments(int argc, char **argv, string *exe, string *inputDir, string *outputDir, int *resolution, string *format, int *framerate);
void idle();
string getExecutablePath(string exe);
void SDL_Die(const char *msg);
void SDL_MainLoop();

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

    parseArguments(argc, argv, &exePath, &cubeDataDir, &equirectDataDir, &hResolution, &outFormat, &videoFrameRate);

    struct stat info;
    if (stat(cubeDataDir.c_str(), &info) != 0) {
        printf("\"%s\" does not exist or cannot be accessed, please specify directory with cubemap images\n", cubeDataDir.c_str());
        return 0;
    }
    else if (!(info.st_mode & S_IFDIR)) {
        printf("\"%s\" is not a directory, please specify directory with cubemap images\n", cubeDataDir.c_str());
        return 0;
    }


    // Initialize SDL's video subsystem (or die on error)
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        SDL_Die("Unable to initialize SDL");

    // Initialize GL attributes
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Declare minimum OpenGL version - 3.2
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // Create our window centered at initial resolution
    mainwindow = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 128, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!mainwindow)
        SDL_Die("Unable to create window");

    maincontext = SDL_GL_CreateContext(mainwindow);

    const unsigned char* glVersion = glGetString(GL_VERSION);
    const unsigned char* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("Using OpenGL %s, GLSL %s\n", glVersion, glslVersion);

    string imgFormat = outFormat == "mp4" ? "jpg" : outFormat;

    renderer = new cube2equirect(mainwindow, exePath);
    renderer->initGL(cubeDataDir, equirectDataDir, hResolution, imgFormat);
    renderer->render();
    idle();

    SDL_MainLoop();
    //SDL_Quit();

    return 0;
}

void parseArguments(int argc, char **argv, string *exe, string *inputDir, string *outputDir, int *resolution, string *format, int *framerate) {
    *exe = getExecutablePath(argv[0]);
    *outputDir = *exe + "output/";
    *resolution = 3840;
    *format = "";
    *framerate = 24;
    bool hasInput = false;

    if (argc >= 3) {
        if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--input") == 0) {
            *inputDir = argv[2];
            hasInput = true;
        }
        else if (strcmp(argv[1], "-o") == 0 || strcmp(argv[1], "--output") == 0) {
            *outputDir = argv[2];
        }
        else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--h-resolution") == 0) {
            int res = atoi(argv[2]);
            if (res > 1) *resolution = res;
        }
        else if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--format") == 0) {
            *format = argv[2];
        }
        else if (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--framerate") == 0) {
            int fr = atoi(argv[2]);
            if (fr > 1) *framerate = fr;
        }
    }
    if (argc >= 5) {
        if (strcmp(argv[3], "-i") == 0 || strcmp(argv[3], "--input") == 0) {
            *inputDir = argv[4];
            hasInput = true;
        }
        else if (strcmp(argv[3], "-o") == 0 || strcmp(argv[3], "--output") == 0) {
            *outputDir = argv[4];
        }
        else if (strcmp(argv[3], "-h") == 0 || strcmp(argv[3], "--h-resolution") == 0) {
            int res = atoi(argv[4]);
            if (res > 1) *resolution = res;
        }
        else if (strcmp(argv[3], "-f") == 0 || strcmp(argv[3], "--format") == 0) {
            *format = argv[4];
        }
        else if (strcmp(argv[3], "-r") == 0 || strcmp(argv[3], "--framerate") == 0) {
            int fr = atoi(argv[4]);
            if (fr > 1) *framerate = fr;
        }
    }
    if (argc >= 7) {
        if (strcmp(argv[5], "-i") == 0 || strcmp(argv[5], "--input") == 0) {
            *inputDir = argv[6];
            hasInput = true;
        }
        else if (strcmp(argv[5], "-o") == 0 || strcmp(argv[5], "--output") == 0) {
            *outputDir = argv[6];
        }
        else if (strcmp(argv[5], "-h") == 0 || strcmp(argv[5], "--h-resolution") == 0) {
            int res = atoi(argv[6]);
            if (res > 1) *resolution = res;
        }
        else if (strcmp(argv[5], "-f") == 0 || strcmp(argv[5], "--format") == 0) {
            *format = argv[6];
        }
        else if (strcmp(argv[5], "-r") == 0 || strcmp(argv[5], "--framerate") == 0) {
            int fr = atoi(argv[6]);
            if (fr > 1) *framerate = fr;
        }
    }
    if (argc >= 9) {
        if (strcmp(argv[7], "-i") == 0 || strcmp(argv[7], "--input") == 0) {
            *inputDir = argv[8];
            hasInput = true;
        }
        else if (strcmp(argv[7], "-o") == 0 || strcmp(argv[7], "--output") == 0) {
            *outputDir = argv[8];
        }
        else if (strcmp(argv[7], "-h") == 0 || strcmp(argv[7], "--h-resolution") == 0) {
            int res = atoi(argv[8]);
            if (res > 1) *resolution = res;
        }
        else if (strcmp(argv[7], "-f") == 0 || strcmp(argv[7], "--format") == 0) {
            *format = argv[8];
        }
        else if (strcmp(argv[7], "-r") == 0 || strcmp(argv[7], "--framerate") == 0) {
            int fr = atoi(argv[8]);
            if (fr > 1) *framerate = fr;
        }
    }
    if (argc >= 11) {
        if (strcmp(argv[9], "-i") == 0 || strcmp(argv[9], "--input") == 0) {
            *inputDir = argv[10];
            hasInput = true;
        }
        else if (strcmp(argv[9], "-o") == 0 || strcmp(argv[9], "--output") == 0) {
            *outputDir = argv[10];
        }
        else if (strcmp(argv[9], "-h") == 0 || strcmp(argv[9], "--h-resolution") == 0) {
            int res = atoi(argv[10]);
            if (res > 1) *resolution = res;
        }
        else if (strcmp(argv[9], "-f") == 0 || strcmp(argv[9], "--format") == 0) {
            *format = argv[10];
        }
        else if (strcmp(argv[9], "-r") == 0 || strcmp(argv[9], "--framerate") == 0) {
            int fr = atoi(argv[10]);
            if (fr > 1) *framerate = fr;
        }
    }

    if (!hasInput) {
        printf("please specify an input directory with cubemap images\n");
        SDL_Quit();
        exit(0);
    }
}

void idle() {
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
}

string getExecutablePath(string exe) {
    int sep = exe.rfind('/');
    return exe.substr(0, sep+1);
}

void finishAndQuit() {
    if (outFormat == "mp4") {
        char ffCmd[512];
        int outFrameRate = videoFrameRate < 24 ? 24 : videoFrameRate;
#ifdef _WIN32
        string oNull = "NUL";
#else
        string oNull = "/dev/null";
#endif
        sprintf(ffCmd, "ffmpeg -y -start_number 0 -r %d -i \"%sequirect_%%06d.jpg\" -r %d -c:v libx264 -pix_fmt yuv420p \"%sequirect.mp4\" > %s 2>&1", videoFrameRate, equirectDataDir.c_str(), outFrameRate, equirectDataDir.c_str(), oNull.c_str());
        int e = system(ffCmd);
        if (e != 0) {
            printf("command:\n\n%s\n\ndid not execute successfully (exit code %d).\n", ffCmd, e);
            printf("jpeg image sequence saved instead.\n");
        }
        else {
            int i = 0;
            char imgFile[256];
            do {
                sprintf(imgFile, "%sequirect_%06d.jpg", equirectDataDir.c_str(), i);
                i++;
            } while (remove(imgFile) == 0);
        }
    }

    SDL_Quit();
    exit(0);
}

void SDL_Die(const char *msg) {
    printf("%s: %s\n", msg, SDL_GetError());
    SDL_Quit();
    exit(1);
}

void SDL_MainLoop() {
    SDL_Event event;
    while (true) {
        SDL_WaitEvent(&event);
        do {
            switch (event.type) {
                case SDL_USEREVENT:
                    if (renderer->hasMoreFrames()) {
                        renderer->updateCubeTextures();
                        renderer->render();
                        idle();
                    }
                    else {
                        finishAndQuit();
                    }
                    break;
                case SDL_QUIT:
                    SDL_Quit();
                    exit(0);
                    break;
                default:
                    break;
            }
        } while (SDL_PollEvent(&event));
    }
}
