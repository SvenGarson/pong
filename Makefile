# Choose compiler
CC = gcc

# Choose compilation unit source files
OBJS = source/*.c

# Compiler flags
COMPILER_FLAGS =

# Linker flags
LINKER_FLAGS = -lm -lGL -lGLU -lSDL2 -lSDL2_image -lSDL2_mixer	

# Executable name
OBJ_NAME = pong

# Build directory
BUILD_DIR = build

# Header directory
INCLUDE_DIR = include

# Targets
compile: $(OBJS)
	$(CC) -I$(INCLUDE_DIR) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(BUILD_DIR)/$(OBJ_NAME)

run:
	$(BUILD_DIR)/$(OBJ_NAME)

compile_and_run: compile run