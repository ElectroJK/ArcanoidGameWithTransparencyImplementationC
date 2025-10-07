CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 -Wall -Iinclude -DGLFW_INCLUDE_NONE
CFLAGS = -std=c11 -Wall -Iinclude -DGLFW_INCLUDE_NONE
LDFLAGS = -Llib -lglfw3 -lopengl32 -lgdi32

SRC_CPP = src/Ball.cpp src/Game.cpp src/Renderer.cpp src/Paddle.cpp src/main.cpp
SRC_C = src/glad.c
OBJ = $(SRC_CPP:.cpp=.o) $(SRC_C:.c=.o)
TARGET = ArcanoidPrototype.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o
	rm -f $(TARGET)
