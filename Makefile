SDL3_DIR := $(HOME)/Downloads/SDL3-3.4.0

%_sdl2.exe: %_sdl2.cc
	$(CXX) $< -Wall $(shell sdl2-config --cflags --libs) -o $@

%_sdl3.exe: %_sdl3.cc
	$(CXX) $< -Wall -o $@ -I$(SDL3_DIR)/include $(SDL3_DIR)/build/libSDL3.a

clean:
	rm -f *.exe *.o *.html *.js *.wasm

install_emsdk:
	git clone https://github.com/emscripten-core/emsdk
	cd emsdk
	./emsdk install latest
	./emsdk activate latest

install_compiledb:
	python3 -m venv venv
	. venv/bin/activate
	pip install compiledb

install_sdl3:
	wget https://github.com/libsdl-org/SDL/releases/download/release-3.4.0/SDL3-3.4.0.tar.gz
	tar xvf SDL3-3.4.0.tar.gz
	cd SDL3-3.4.0
	cmake -S . -B build -DSDL_STATIC=ON
	cmake --build build

%_sdl2.html: %_sdl2.cc
	. ./emsdk/emsdk_env.sh
	emcc -o $@ $< -sUSE_SDL=2

%_sdl3.html: %_sdl3.cc
	. ./emsdk/emsdk_env.sh
	emcc -o $@ $< -sUSE_SDL=3

serve: index.html
	python3 -m http.server

compile_commands.json: Makefile
	. ./venv/bin/activate
	compiledb make all

all: example_cube_sdl2.exe example_cube_sdl3.exe

.PHONY: serve install_emsdk clean install_compiledb all run
