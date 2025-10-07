#pragma once
#include "Renderer.h"

class Paddle {
public:
    Paddle(float x, float y, float w, float h);
    void Update(bool left, bool right, float dt);
    void Draw(Renderer& renderer);
    float GetX() const;
    float GetY() const;
    float GetW() const;
    float GetH() const;
private:
    float x, y, w, h;
    float speed;
};


