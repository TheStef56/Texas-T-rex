SDL_PREFIX=$(PWD)/SDL2_ttf/external/SDL

all:
	cmake -S ./SDL2 -B ./SDL2/build
	cmake --build ./SDL2/build
	cmake --install ./SDL2/build --prefix ./SDL2_ttf/external/SDL
	cd ./SDL2_ttf/ && autoreconf -f -i && ./configure --with-sdl-prefix="${SDL_PREFIX}"
	cd ./SDL2_ttf/ && make -j16
	cd ./SDL2_image/ && autoreconf -f -i && ./configure --with-sdl-prefix="${SDL_PREFIX}"
	cd ./SDL2_image/ && make -j16
	cd ./SDL2_mixer/ && ./configure --with-sdl-prefix="${SDL_PREFIX}"
	cd ./SDL2_mixer/ && make -j16
	rm -rf ../lib ../include
	cp ./SDL2_ttf/.libs/libSDL2_ttf.a ../lib/
	cp ./SDL2_image/.libs/libSDL2_image.a ../lib/
	cp ./SDL2_mixer/build/\.libs/libSDL2_mixer.a ../lib/
	cp ./SDL2_ttf/external/SDL/lib/libSDL2.a ../lib/
	cp ./SDL2_ttf/external/SDL/lib/libSDL2main.a ../lib/
	cp ./SDL2_ttf/external/SDL/include/SDL2/* ../include/SDL2/