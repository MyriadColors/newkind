#
# Makefile for Elite - The New Kind (Windows Jom / NMake Build)
#

CC = clang
CFLAGS = -std=c23 -O2 -Wall -Wextra -pedantic -Ideps/raylib/src -D_CRT_SECURE_NO_WARNINGS
LIBS = deps/raylib/src/libraylib.a -lopengl32 -lgdi32 -lwinmm -luser32 -lshell32
EXEC = newkind.exe

RAYLIB_VERSION = 6.0
RAYLIB_DIR = deps\raylib
RAYLIB_SRC = deps\raylib\src
RAYLIB_LIB = deps\raylib\src\libraylib.a

RAYLIB_OBJS = $(RAYLIB_SRC)\rcore.o $(RAYLIB_SRC)\rshapes.o $(RAYLIB_SRC)\rtextures.o \
              $(RAYLIB_SRC)\rtext.o $(RAYLIB_SRC)\rmodels.o $(RAYLIB_SRC)\raudio.o \
              $(RAYLIB_SRC)\rglfw.o

RAYLIB_CFLAGS = -std=c99 -O2 -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 -D_CRT_SECURE_NO_WARNINGS \
                -I$(RAYLIB_SRC) -I$(RAYLIB_SRC)/external/glfw/include

OBJS = game_state.o gfx.o main.o docked.o elite.o \
       intro.o planet.o shipdata.o shipface.o sound.o space.o \
       swat.o threed.o vector.o random.o trade.o options.o \
       stars.o missions.o pilot.o file.o keyboard.o

all: $(EXEC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure Raylib static library and headers exist before compiling any project object
$(OBJS): $(RAYLIB_LIB)

$(EXEC): $(OBJS) $(RAYLIB_LIB)
	$(CC) -o $(EXEC) $(OBJS) $(LIBS)

$(RAYLIB_LIB):
	@if not exist "$(RAYLIB_DIR)" git clone --depth 1 --branch $(RAYLIB_VERSION) https://github.com/raysan5/raylib.git $(RAYLIB_DIR)
	$(CC) $(RAYLIB_CFLAGS) -c $(RAYLIB_SRC)/rcore.c -o $(RAYLIB_SRC)/rcore.o
	$(CC) $(RAYLIB_CFLAGS) -c $(RAYLIB_SRC)/rshapes.c -o $(RAYLIB_SRC)/rshapes.o
	$(CC) $(RAYLIB_CFLAGS) -c $(RAYLIB_SRC)/rtextures.c -o $(RAYLIB_SRC)/rtextures.o
	$(CC) $(RAYLIB_CFLAGS) -c $(RAYLIB_SRC)/rtext.c -o $(RAYLIB_SRC)/rtext.o
	$(CC) $(RAYLIB_CFLAGS) -c $(RAYLIB_SRC)/rmodels.c -o $(RAYLIB_SRC)/rmodels.o
	$(CC) $(RAYLIB_CFLAGS) -c $(RAYLIB_SRC)/raudio.c -o $(RAYLIB_SRC)/raudio.o
	$(CC) $(RAYLIB_CFLAGS) -c $(RAYLIB_SRC)/rglfw.c -o $(RAYLIB_SRC)/rglfw.o
	llvm-ar rcs $(RAYLIB_LIB) $(RAYLIB_OBJS)

clean:
	-cmd /c "del /f /q *.o $(EXEC) 2>nul"
	-cmd /c "if exist $(RAYLIB_SRC) del /f /q $(RAYLIB_SRC)\*.o $(RAYLIB_LIB) 2>nul"

clean-deps:
	-cmd /c "rmdir /s /q $(RAYLIB_DIR) 2>nul"

distclean: clean clean-deps

# Per-object header dependencies
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
