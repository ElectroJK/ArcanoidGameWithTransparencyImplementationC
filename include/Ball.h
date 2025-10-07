#pragma once
#include "Renderer.h"

class Ball {
public:
    Ball(float x, float y, float r);
    void Update(float dt);
    void Draw(Renderer& renderer);
    float GetX() const;
    float GetY() const;
    float GetRadius() const;
    float GetVX() const;
    float GetVY() const;
    void SetX(float nx);
    void SetY(float ny);
    void SetVX(float nvx);
    void SetVY(float nvy);
private:
    float x, y, radius;
    float vx, vy;
};
