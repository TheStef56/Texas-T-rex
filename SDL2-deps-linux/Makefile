SDL_PREFIX=$(PWD)/SDL2_ttf/external/SDL

SDL_VIDEO_FLAGS =
ifeq ($(XDG_SESSION_TYPE),wayland)
    SDL_VIDEO_FLAGS = -DSDL_VIDEO_WAYLAND=ON -DSDL_VIDEO_X11=OFF
else ifeq ($(XDG_SESSION_TYPE),x11)
    SDL_VIDEO_FLAGS = -DSDL_VIDEO_WAYLAND=OFF -DSDL_VIDEO_X11=ON
else
    SDL_VIDEO_FLAGS = -DSDL_VIDEO_WAYLAND=ON -DSDL_VIDEO_X11=ON
endif

all:
	cmake -S ./SDL2 -B ./SDL2/build $(SDL_VIDEO_FLAGS)
	cmake --build ./SDL2/build
	cmake --install ./SDL2/build --prefix ./SDL2_ttf/external/SDL
	cd ./SDL2_ttf/ && autoreconf -f -i && ./configure --with-sdl-prefix="${SDL_PREFIX}"
	cd ./SDL2_ttf/ && make
	cd ./SDL2_image/ && autoreconf -f -i && ./configure --with-sdl-prefix="${SDL_PREFIX}"
	cd ./SDL2_image/ && make
	cd ./SDL2_mixer/ && ./configure --with-sdl-prefix="${SDL_PREFIX}"
	cd ./SDL2_mixer/ && make
	cp ./SDL2_ttf/.libs/libSDL2_ttf.a ../lib/
	cp ./SDL2_image/.libs/libSDL2_image.a ../lib/
	cp ./SDL2_mixer/build/\.libs/libSDL2_mixer.a ../lib/
	cp ./SDL2_ttf/external/SDL/lib/libSDL2.a ../lib/
	cp ./SDL2_ttf/external/SDL/lib/libSDL2main.a ../lib/
	cp ./SDL2_ttf/external/SDL/include/SDL2/* ../include/SDL2/