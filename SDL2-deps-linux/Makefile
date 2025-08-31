SDL_PREFIX=$(PWD)/SDL2_ttf/external/SDL

all:
	cd ./SDL2 && ./autogen.sh
	cd ./SDL2 && ./configure --prefix="$(SDL_PREFIX)"
	cd ./SDL2 && $(MAKE) && $(MAKE) install
	cd ./SDL2_ttf/ && ./autogen.sh  && ./configure --with-sdl-prefix="$(SDL_PREFIX)"
	cd ./SDL2_ttf/ && $(MAKE)
	cd ./SDL2_image/ && ./autogen.sh && ./configure --with-sdl-prefix="$(SDL_PREFIX)"
	cd ./SDL2_image/ && $(MAKE)
	cd ./SDL2_mixer/ && ./autogen.sh && ./configure --with-sdl-prefix="$(SDL_PREFIX)"
	cd ./SDL2_mixer/ && $(MAKE)
	cp ./SDL2_ttf/.libs/libSDL2_ttf.a ../lib/
	cp ./SDL2_image/.libs/libSDL2_image.a ../lib/
	cp ./SDL2_mixer/build/.libs/libSDL2_mixer.a ../lib/
	cp ./SDL2_ttf/external/SDL/lib/libSDL2.a ../lib/
	cp ./SDL2_ttf/external/SDL/lib/libSDL2main.a ../lib/
	cp ./SDL2_ttf/external/SDL/include/SDL2/* ../include/SDL2/
