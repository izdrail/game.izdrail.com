# CMake Wrapper Makefile

.PHONY: all build clean run help

all: build

build:
	mkdir -p build && cd build && cmake .. && cmake --build .

clean:
	rm -rf build
	rm -f game Drone-Shooter

run: build
	./game

help:
	@echo "Hardened Build System (via CMake)"
	@echo "Usage:"
	@echo "  make        Build the project"
	@echo "  make run    Build and run the project"
	@echo "  make clean  Remove build artifacts"
