#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Paddle.h"

Paddle::Paddle(float x, float y, float w, float h) : x(x), y(y), w(w), h(h), speed(1.2f) {}

void Paddle::Update(bool left, bool right, float dt) {
    if (left) x -= speed * dt;
    if (right) x += speed * dt;
    if (x - w < -1.0f) x = -1.0f + w;
    if (x + w > 1.0f) x = 1.0f - w;
}

void Paddle::Draw(Renderer& renderer) {
    float verts[] = { -w, -h,  w, -h,  w,  h, -w,  h };
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glUseProgram(renderer.GetShaderProgram());
    int colorLoc = glGetUniformLocation(renderer.GetShaderProgram(), "color");
    glUniform4f(colorLoc, 0.2f, 0.8f, 1.0f, 1.0f);
    renderer.UseTexture(false);
    renderer.SetOffset(x, y);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

float Paddle::GetX() const { return x; }
float Paddle::GetY() const { return y; }
float Paddle::GetW() const { return w; }
float Paddle::GetH() const { return h; }
void Paddle::SetW(float newW) { w = newW; }
float Paddle::GetSpeed() const { return speed; }
void Paddle::SetSpeed(float s) { speed = s; }
