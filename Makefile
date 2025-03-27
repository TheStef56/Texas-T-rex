
ifeq ($(OS), Windows_NT)
	CMD = gcc -o trex main.c -g -I./include -L./lib/win32/x64 -lSDL2main -lSDL2 -lgdi32 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm &&\
	copy .\lib\win32\x64 .
else
	CMD = cp -r SDL2-deps-linux build &&\
	cd build && make &&\
	cc -o trex main.c -g -Wall -Wextra -I./include -L./lib -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

endif

all:
	$(CMD)

