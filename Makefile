############
CC= gcc
CXX= g++
CCFLAGS= -g
CXXFLAGS= -g

# include directories and libraries
INC= -I./include
LIB= -lEGL -lGL

# object files have corresponding source files
OBJDIR= objs
C_SOURCES = $(wildcard src/*.c)
CXX_SOURCES = $(wildcard src/m*.cpp)
C_OBJS= $(patsubst src/%.c, $(OBJDIR)/%.o, $(C_SOURCES))
CXX_OBJS= $(patsubst src/%.cpp, $(OBJDIR)/%.o, $(CXX_SOURCES))

# bin output
BINDIR= bin
EXEC= $(addprefix $(BINDIR)/, cube2equirect)


mkdirs:= $(shell mkdir -p $(OBJDIR) $(BINDIR))



# BUILD EVERYTHING
all: $(EXEC)

$(EXEC): $(C_OBJS) $(CXX_OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(C_OBJS) $(CXX_OBJS) $(LIB)

$(OBJDIR)/%.o: src/%.c
	$(CC) -c $(CCFLAGS) -o $@ $< $(INC)

$(OBJDIR)/%.o: src/%.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< $(INC)


# REMOVE OLD FILES
clean:
	rm -f $(EXEC) $(C_OBJS) $(CXX_OBJS)

