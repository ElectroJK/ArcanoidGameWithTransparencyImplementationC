#pragma once
#include "Renderer.h"
#include "Ball.h"
#include "Paddle.h"

class Game {
public:
    Game(int width, int height, const char* title);
    void Run();
private:
    Renderer renderer;
    Ball ball;
    Paddle paddle;
    bool running;
    double lastTime;
    void ProcessInput();
};
