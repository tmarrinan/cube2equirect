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

void parseArguments(int argc, char **argv, string *inputDir, string *outputDir, int *resolution);
void idle();
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
		printf("    -r, --resolution-h <NUMBER>  horizontal resolution of output images [Default: 3840]\n");
		printf("\n");
		return 0;
	}

	string cubeDataDir;
	string equirectDataDir;
	int hResolution;
	parseArguments(argc, argv, &cubeDataDir, &equirectDataDir, &hResolution);

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

	renderer = new cube2equirect(mainwindow);
	renderer->initGL(cubeDataDir, equirectDataDir, hResolution);
	renderer->render();
	idle();

	SDL_MainLoop();
	//SDL_Quit();

	return 0;
}

void parseArguments(int argc, char **argv, string *inputDir, string *outputDir, int *resolution) {
	*outputDir = "output/";
	*resolution = 3840;
	bool hasInput = false;

	if (argc >= 3) {
		if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--input") == 0) {
			*inputDir = argv[2];
			hasInput = true;
		}
		else if (strcmp(argv[1], "-o") == 0 || strcmp(argv[1], "--output") == 0) {
			*outputDir = argv[2];
		}
		else if (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--resolution-h") == 0) {
			*resolution = atoi(argv[2]);
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
		else if (strcmp(argv[3], "-r") == 0 || strcmp(argv[3], "--resolution-h") == 0) {
			*resolution = atoi(argv[4]);
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
		else if (strcmp(argv[5], "-r") == 0 || strcmp(argv[5], "--resolution-h") == 0) {
			*resolution = atoi(argv[6]);
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
						SDL_Quit();
						exit(0);
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