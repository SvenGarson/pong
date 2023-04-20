# Choose a compiler
CC=gcc

# Set include directory to look for project headers
IDIR =../include
CFLAGS=-I$(IDIR)

# Directories for compilation unit objects and linked libraries
ODIR=obj
LDIR =../lib

# Linked libraries
LIBS=-lSDL2 -lGL -lGLU -lSDL2_image

# Header that source files depend on TODO-GS: Add all dependencies
_DEPS = pong_bool.h text_renderer.h texture_loader.h batcher.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# Compilation unit object files to build
_OBJ = pong.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

# Re-generate compilation unit object files when corresponding headers change
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Targets
compile: $(OBJ)
	clear
	$(CC) -o ../build/pong $^ $(CFLAGS) $(LIBS)

run:
	../build/pong

compile_and_run: compile run

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~