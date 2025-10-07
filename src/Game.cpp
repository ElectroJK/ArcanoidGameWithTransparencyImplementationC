#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include <iostream>
#include <cmath>

Game::Game(int width, int height, const char* title)
    : renderer(width, height, title), running(true), ball(0.0f, 0.0f, 0.05f), paddle(0.0f, -0.8f, 0.2f, 0.05f), lastTime(glfwGetTime()) {}

void Game::Run() {
    while (running && !renderer.ShouldClose()) {
        ProcessInput();
        renderer.Clear();
        double now = glfwGetTime();
        float dt = static_cast<float>(now - lastTime);
        lastTime = now;
        ball.Update(dt);
        {
            float bx = ball.GetX();
            float by = ball.GetY();
            float br = ball.GetRadius();
            float px = paddle.GetX();
            float py = paddle.GetY();
            float pw = paddle.GetW();
            float ph = paddle.GetH();
            float closestX = bx;
            if (bx < px - pw) closestX = px - pw;
            else if (bx > px + pw) closestX = px + pw;
            float closestY = by;
            if (by < py - ph) closestY = py - ph;
            else if (by > py + ph) closestY = py + ph;
            float dx = bx - closestX;
            float dy = by - closestY;
            float dist2 = dx*dx + dy*dy;
            if (dist2 <= br*br && ball.GetVY() < 0.0f) {
                float hitPos = (bx - px) / pw;
                if (hitPos < -1.0f) hitPos = -1.0f;
                if (hitPos > 1.0f) hitPos = 1.0f;
                float speed = sqrtf(ball.GetVX()*ball.GetVX() + ball.GetVY()*ball.GetVY());
                float minDeg = 20.0f * 3.1415926535f / 180.0f;
                float maxDeg = 160.0f * 3.1415926535f / 180.0f;
                float t = (hitPos + 1.0f) * 0.5f;
                float theta = minDeg + t * (maxDeg - minDeg);
                float nxvx = speed * cosf(theta);
                float nxvy = speed * sinf(theta);
                ball.SetVX(nxvx);
                ball.SetVY(nxvy);
                ball.SetY(py + ph + br + 0.001f);
            }
        }
        ball.Draw(renderer);
        paddle.Draw(renderer);
        renderer.Display();
    }
}

void Game::ProcessInput() {
    if (renderer.IsKeyPressed(GLFW_KEY_ESCAPE))
        running = false;
    bool left = renderer.IsKeyPressed(GLFW_KEY_LEFT) || renderer.IsKeyPressed(GLFW_KEY_A);
    bool right = renderer.IsKeyPressed(GLFW_KEY_RIGHT) || renderer.IsKeyPressed(GLFW_KEY_D);
    double now = glfwGetTime();
    float dt = static_cast<float>(now - lastTime);
    if (left || right) paddle.Update(left, right, dt);
}
