#
# Makefile for Elite - The New Kind (Raylib Port)
#

# Detect OS
ifeq ($(OS),Windows_NT)
    PLATFORM_OS ?= WINDOWS
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM_OS ?= LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        PLATFORM_OS ?= OSX
    endif
endif

CC ?= clang
CFLAGS ?= -std=c23 -O2 -Wall -Wextra -pedantic

# Raylib dependency settings
RAYLIB_VERSION ?= 6.0
RAYLIB_DIR ?= deps/raylib
RAYLIB_SRC ?= $(RAYLIB_DIR)/src
RAYLIB_LIB ?= $(RAYLIB_SRC)/libraylib.a

CFLAGS += -I$(RAYLIB_SRC)

# Platform-specific libraries for Raylib static linking
ifeq ($(PLATFORM_OS),WINDOWS)
    LIBS = $(RAYLIB_LIB) -lopengl32 -lgdi32 -lwinmm -lm
    EXEC_EXT = .exe
else ifeq ($(PLATFORM_OS),OSX)
    LIBS = $(RAYLIB_LIB) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lm
    EXEC_EXT =
else
    LIBS = $(RAYLIB_LIB) -lGL -lm -lpthread -ldl -lrt -lX11
    EXEC_EXT =
endif

EXEC = newkind$(EXEC_EXT)

OBJS = game_state.o gfx.o main.o docked.o elite.o \
       intro.o planet.o shipdata.o shipface.o sound.o space.o \
       swat.o threed.o vector.o random.o trade.o options.o \
       stars.o missions.o pilot.o file.o keyboard.o

all: $(EXEC)

# Rule to clone raylib if missing
$(RAYLIB_DIR):
	@echo "Fetching Raylib $(RAYLIB_VERSION)..."
	git clone --depth 1 --branch $(RAYLIB_VERSION) https://github.com/raysan5/raylib.git $(RAYLIB_DIR)

# Rule to compile raylib static library
$(RAYLIB_LIB): $(RAYLIB_DIR)
	@echo "Building Raylib $(RAYLIB_VERSION)..."
	$(MAKE) -C $(RAYLIB_SRC) CC=$(CC) RAYLIB_LIBTYPE=STATIC

# Ensure raylib is built before any project object file compiles
$(OBJS): $(RAYLIB_LIB)

clean:
	rm -f *.o $(EXEC) newkind newkind-raylib newkind.exe

clean-deps:
	rm -rf $(RAYLIB_DIR)

distclean: clean clean-deps

.SUFFIXES : .c .o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC): $(OBJS) $(RAYLIB_LIB)
	$(CC) -o $(EXEC) $(OBJS) $(LIBS)

gfx.o: gfx.c config.h elite.h planet.h gfx.h
main.o: main.c config.h elite.h planet.h gfx.h docked.h \
	intro.h shipdata.h shipface.h space.h main.h pilot.h file.h keyboard.h
docked.o: docked.c config.h elite.h planet.h gfx.h
elite.o: elite.c config.h elite.h planet.h vector.h shipdata.h
intro.o: intro.c space.h config.h elite.h planet.h gfx.h vector.h \
	shipdata.h shipface.h threed.h
planet.o: planet.c config.h elite.h planet.h
shipdata.o: shipdata.c shipdata.h vector.h
shipface.o: shipface.c config.h elite.h planet.h shipface.h gfx.h
threed.o: threed.c space.h config.h elite.h planet.h gfx.h vector.h shipdata.h \
	shipface.h threed.h
vector.o: vector.c config.h vector.h
sound.o: sound.c sound.h
space.o: space.c space.h vector.h config.h elite.h planet.h \
	gfx.h docked.h intro.h shipdata.h shipface.h main.h random.h
swat.o: swat.c swat.h elite.h config.h main.h gfx.h shipdata.h \
	random.h pilot.h
random.o: random.c random.h
trade.o: trade.c trade.h elite.h config.h
options.o: options.c options.h elite.h config.h gfx.h file.h
stars.o: stars.c stars.h elite.h config.h gfx.h random.h
missions.o: missions.c missions.h config.h elite.h gfx.h planet.h main.h \
	vector.h space.h
pilot.o: pilot.c pilot.h config.h elite.h gfx.h vector.h space.h main.h
file.o: file.c file.h config.h elite.h
keyboard.o: keyboard.c keyboard.h elite.h gfx.h
game_state.o: game_state.c game_state.h planet.h trade.h vector.h shipdata.h
