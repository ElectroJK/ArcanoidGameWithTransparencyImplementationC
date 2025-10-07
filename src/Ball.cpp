#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Ball.h"

Ball::Ball(float x, float y, float r) : x(x), y(y), radius(r), vx(0.6f), vy(0.6f) {}

void Ball::Update(float dt) {
    x += vx * dt;
    y += vy * dt;
    if (x - radius < -1.0f) { x = -1.0f + radius; vx = -vx; }
    if (x + radius > 1.0f) { x = 1.0f - radius; vx = -vx; }
    if (y - radius < -1.0f) { y = -1.0f + radius; vy = -vy; }
    if (y + radius > 1.0f) { y = 1.0f - radius; vy = -vy; }
}

void Ball::Draw(Renderer& renderer) {
    float vertices[] = { -radius, -radius, radius, -radius, radius, radius, -radius, radius };
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glUseProgram(renderer.GetShaderProgram());
    int colorLoc = glGetUniformLocation(renderer.GetShaderProgram(), "color");
    glUniform4f(colorLoc, 1.0f, 0.3f, 0.3f, 1.0f);
    renderer.SetOffset(x, y);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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
