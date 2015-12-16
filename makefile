############
MACHINE= $(shell uname -s)

ifeq ($(MACHINE),Darwin)
	OPENGL_INC= -FOpenGL
	OPENGL_LIB= -framework OpenGL
	SDL_INC= `sdl2-config --cflags`
	SDL_LIB= `sdl2-config --libs` -lSDL2_image
	IMG_INC= -I/usr/local/include
	IMG_LIB= -lpng -ljpeg
else
	OPENGL_INC= -I/usr/X11R6/include
	OPENGL_LIB= -I/usr/lib64 -lGL -lGLU
	SDL_INC= `sdl2-config --cflags`
	SDL_LIB= `sdl2-config --libs` -lSDL2_image
	IMG_INC= -I/usr/local/include
	IMG_LIB= -lpng -ljpeg
endif

# object files have corresponding source files
OBJDIR= objs
OBJS= $(addprefix $(OBJDIR)/, main.o cube2equirect.o)
CXX= g++
COMPILER_FLAGS= -g
INCLUDE= $(SDL_INC) $(IMG_INC) $(OPENGL_INC) 
LIBS= $(SDL_LIB) $(IMG_LIB) $(OPENGL_LIB)

EXEC= cube2equirect

$(EXEC): $(OBJS)
	$(CXX) $(COMPILER_FLAGS) -o $(EXEC) $(OBJS) $(LIBS)

$(OBJDIR)/%.o: src/%.cpp
	$(CXX) -c $(COMPILER_FLAGS) -o $@ $< $(INCLUDE)

all: $(OBJS)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)
	
clean:
	rm -f $(EXEC) $(OBJS)
