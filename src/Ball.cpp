#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Ball.h"
#include <vector>
#include <cmath>

Ball::Ball(float x, float y, float r) : x(x), y(y), radius(r), vx(0.6f), vy(0.6f) {}

void Ball::Update(float dt) {
    x += vx * dt;
    y += vy * dt;
    if (x - radius < -1.0f) { x = -1.0f + radius; vx = -vx; }
    if (x + radius > 1.0f) { x = 1.0f - radius; vx = -vx; }
}

void Ball::Draw(Renderer& renderer) {
    const int segments = 32;
    std::vector<float> verts;
    verts.reserve((segments + 2) * 2);
    verts.push_back(0.0f); verts.push_back(0.0f);
    for (int i = 0; i <= segments; ++i) {
        float ang = (float)i / segments * 6.28318530718f;
        verts.push_back(std::cos(ang) * radius);
        verts.push_back(std::sin(ang) * radius);
    }
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glUseProgram(renderer.GetShaderProgram());
    int colorLoc = glGetUniformLocation(renderer.GetShaderProgram(), "color");
    glUniform4f(colorLoc, 1.0f, 0.3f, 0.3f, 1.0f);
    renderer.UseTexture(false);
    renderer.SetOffset(x, y);
    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

float Ball::GetX() const { return x; }
float Ball::GetY() const { return y; }
float Ball::GetRadius() const { return radius; }
float Ball::GetVX() const { return vx; }
float Ball::GetVY() const { return vy; }
void Ball::SetX(float nx) { x = nx; }
void Ball::SetY(float ny) { y = ny; }
void Ball::SetVX(float nvx) { vx = nvx; }
void Ball::SetVY(float nvy) { vy = nvy; }
