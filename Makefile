# Compiler
CXX = g++
CC = gcc

# Project Name
TARGET = Drone-Shooter

# Directories
SRC_DIR = src
INC_DIR = include

# Sources
# Find all .cpp and .c files in src/ and the main.cpp in root
SRCS_CPP = $(wildcard $(SRC_DIR)/*.cpp) main.cpp
SRCS_C = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS_CPP:.cpp=.o) $(SRCS_C:.c=.o)

# dependencies
PKG_DEPS = glfw3 assimp freetype2

# Check if pkg-config is available and packages exist
PKG_CONFIG_EXISTS := $(shell command -v pkg-config 2> /dev/null)
ifdef PKG_CONFIG_EXISTS
    PKG_CFLAGS := $(shell pkg-config --cflags $(PKG_DEPS))
    PKG_LIBS := $(shell pkg-config --libs $(PKG_DEPS))
else
    $(warning pkg-config not found. Make sure dependencies are installed and paths are correct.)
endif

# Flags
CXXFLAGS = -std=c++17 -Wall -g -I$(INC_DIR) $(PKG_CFLAGS)
CFLAGS = -Wall -g -I$(INC_DIR) $(PKG_CFLAGS)

# Linker Flags (includes system libraries used in CMakeLists.txt)
LDFLAGS = $(PKG_LIBS) -lGL -lX11 -lpthread -ldl

# Targets
.PHONY: all clean run help

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compile C++ source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the application
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Help
help:
	@echo "Makefile for $(TARGET)"
	@echo "Usage:"
	@echo "  make        Build the project"
	@echo "  make run    Build and run the project"
	@echo "  make clean  Remove build artifacts"
