#
# GNUmakefile for Elite - The New Kind (Linux, macOS, and POSIX GNU Make)
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

ifeq ($(origin CC),default)
    CC = clang
endif
CC ?= clang
CFLAGS ?= -std=c23 -O2 -Wall -Wextra -pedantic

# Raylib dependency settings
RAYLIB_VERSION ?= 6.0
RAYLIB_DIR ?= deps/raylib
RAYLIB_SRC ?= $(RAYLIB_DIR)/src
RAYLIB_LIB ?= $(RAYLIB_SRC)/libraylib.a

# Check for system-wide raylib (via pkg-config on Linux/macOS)
ifeq ($(shell pkg-config --exists raylib 2>/dev/null && echo yes),yes)
    RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib)
    RAYLIB_LIBS   := $(shell pkg-config --libs raylib)
    RAYLIB_DEP    :=
    USE_SYSTEM_RAYLIB = 1
else
    USE_SYSTEM_RAYLIB = 0
    RAYLIB_CFLAGS := -I$(RAYLIB_SRC)
    ifeq ($(PLATFORM_OS),WINDOWS)
        ifeq ($(origin AR),default)
            AR = llvm-ar
        endif
        AR ?= llvm-ar
        CFLAGS += -D_CRT_SECURE_NO_WARNINGS
        RAYLIB_LIBS = $(RAYLIB_LIB) -lopengl32 -lgdi32 -lwinmm -luser32 -lshell32
    else ifeq ($(PLATFORM_OS),OSX)
        AR ?= ar
        RAYLIB_LIBS = $(RAYLIB_LIB) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    else
        AR ?= ar
        GL_LIBS := $(shell pkg-config --libs gl 2>/dev/null || echo "-lGL")
        X11_LIBS := $(shell pkg-config --libs x11 2>/dev/null || echo "-lX11")
        RAYLIB_LIBS = $(RAYLIB_LIB) $(GL_LIBS) $(X11_LIBS)
    endif
    RAYLIB_DEP := $(RAYLIB_LIB)
endif

ifeq ($(PLATFORM_OS),LINUX)
    SYS_LIBS = -lm -lpthread -ldl -lrt
else ifeq ($(PLATFORM_OS),OSX)
    SYS_LIBS = -lm
else
    SYS_LIBS =
endif

CFLAGS += $(RAYLIB_CFLAGS)
LIBS = $(RAYLIB_LIBS) $(SYS_LIBS)

ifeq ($(PLATFORM_OS),WINDOWS)
    EXEC_EXT = .exe
else
    EXEC_EXT =
endif

EXEC = newkind$(EXEC_EXT)

OBJS = game_state.o gfx.o main.o docked.o elite.o \
       intro.o planet.o shipdata.o shipface.o sound.o space.o \
       swat.o threed.o vector.o random.o trade.o options.o \
       stars.o missions.o pilot.o file.o keyboard.o

.PHONY: all clean clean-deps distclean

all: $(EXEC)

# Rule to clone raylib if missing
$(RAYLIB_DIR):
ifeq ($(PLATFORM_OS),WINDOWS)
	@if not exist "$(RAYLIB_DIR)" git clone --depth 1 --branch $(RAYLIB_VERSION) https://github.com/raysan5/raylib.git $(RAYLIB_DIR)
else
	@test -d $(RAYLIB_DIR) || git clone --depth 1 --branch $(RAYLIB_VERSION) https://github.com/raysan5/raylib.git $(RAYLIB_DIR)
endif

# Rule to compile raylib static library
$(RAYLIB_LIB): $(RAYLIB_DIR)
	@echo "Building Raylib $(RAYLIB_VERSION)..."
	$(MAKE) -C $(RAYLIB_SRC) CC=$(CC) AR=$(AR) RAYLIB_LIBTYPE=STATIC

# Ensure raylib is built before any project object file compiles
$(OBJS): $(RAYLIB_DEP)

clean:
ifeq ($(PLATFORM_OS),WINDOWS)
	-@cmd /c "del /f /q *.o $(EXEC) newkind newkind.exe 2>nul" || rm -f *.o $(EXEC) newkind newkind.exe 2>/dev/null || true
	-@if exist "$(RAYLIB_SRC)" cmd /c "del /f /q $(subst /,\,$(RAYLIB_SRC))\*.o $(subst /,\,$(RAYLIB_LIB)) 2>nul" || true
else
	@rm -f *.o $(EXEC) newkind newkind.exe 2>/dev/null || true
	@if [ -d "$(RAYLIB_SRC)" ]; then rm -f $(RAYLIB_SRC)/*.o $(RAYLIB_LIB) 2>/dev/null || true; fi
endif

clean-deps:
ifeq ($(PLATFORM_OS),WINDOWS)
	-@cmd /c "rmdir /s /q $(subst /,\,$(RAYLIB_DIR)) 2>nul" || rm -rf $(RAYLIB_DIR) 2>/dev/null || true
else
	@rm -rf $(RAYLIB_DIR) 2>/dev/null || true
endif

distclean: clean clean-deps

.SUFFIXES : .c .o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC): $(OBJS) $(RAYLIB_DEP)
	$(CC) -o $(EXEC) $(OBJS) $(LIBS)

gfx.o: gfx.c config.h elite.h planet.h gfx.h keyboard.h game_state.h
main.o: main.c config.h elite.h planet.h gfx.h docked.h \
	intro.h shipdata.h shipface.h space.h main.h pilot.h file.h keyboard.h game_state.h
docked.o: docked.c config.h elite.h planet.h gfx.h docked.h game_state.h
elite.o: elite.c config.h elite.h planet.h vector.h shipdata.h game_state.h
intro.o: intro.c space.h config.h elite.h planet.h gfx.h vector.h \
	shipdata.h shipface.h threed.h game_state.h
planet.o: planet.c config.h elite.h planet.h missions.h game_state.h
shipdata.o: shipdata.c shipdata.h vector.h
shipface.o: shipface.c config.h elite.h planet.h shipface.h gfx.h
threed.o: threed.c space.h config.h elite.h planet.h gfx.h vector.h shipdata.h \
	shipface.h threed.h game_state.h
vector.o: vector.c config.h vector.h
sound.o: sound.c sound.h game_state.h
space.o: space.c space.h vector.h config.h elite.h planet.h \
	gfx.h docked.h intro.h shipdata.h shipface.h main.h random.h game_state.h
swat.o: swat.c swat.h elite.h config.h main.h gfx.h shipdata.h \
	random.h pilot.h game_state.h
random.o: random.c random.h game_state.h
trade.o: trade.c trade.h elite.h config.h game_state.h
options.o: options.c options.h elite.h config.h gfx.h file.h keyboard.h game_state.h
stars.o: stars.c stars.h elite.h config.h gfx.h random.h game_state.h
missions.o: missions.c missions.h config.h elite.h gfx.h planet.h main.h \
	vector.h space.h keyboard.h game_state.h
pilot.o: pilot.c pilot.h config.h elite.h gfx.h vector.h space.h main.h game_state.h
file.o: file.c file.h config.h elite.h game_state.h
keyboard.o: keyboard.c keyboard.h elite.h gfx.h game_state.h
game_state.o: game_state.c game_state.h planet.h trade.h vector.h shipdata.h
