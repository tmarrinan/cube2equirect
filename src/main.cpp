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

void idle();
void SDL_Die(const char *msg);
void SDL_MainLoop();

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Please specify directory with cubemap images\n");
		return 0;
	}

	struct stat info;
	string cubeDataDir;
	if (stat(argv[1], &info) != 0) {
		printf( "\"%s\" does not exist or cannot be accessed, please specify directory with cubemap images\n", argv[1]);
		return 0;
	}
	else if (info.st_mode & S_IFDIR) {
		cubeDataDir = argv[1];
	}
	else {
		printf( "\"%s\" is not a directory, please specify directory with cubemap images\n", argv[1]);
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
	renderer->initGL(cubeDataDir);
	renderer->render();
	idle();

	SDL_MainLoop();
	//SDL_Quit();

	return 0;
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
