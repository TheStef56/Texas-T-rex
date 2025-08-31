ARCH_TYPE := $(shell echo %PROCESSOR_ARCHITEW6432%)

ifeq ($(findstring AMD64 , $(ARCH_TYPE)), AMD64)
    ARCH = x64
else
	ARCH = x86
endif

ifeq ($(OS), Windows_NT)
	CMD = gcc -o trex main.c -g -Wall -Wextra -I./include -L./lib/win32/$(ARCH) \
	-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer \
	-lm -lgdi32 -lwinmm -lrpcrt4 -lsetupapi -lole32 -limm32 -lversion -loleaut32 -static
else
	CMD = +cp -r SDL2-deps-linux build &&\
	cd build && make &&\
	cd .. &&\
	cc -o trex main.c -g -Wall -Wextra -I./include -L./lib -lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm -Bstatic

endif

all:
	$(CMD)

