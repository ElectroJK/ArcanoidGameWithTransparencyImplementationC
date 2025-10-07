#pragma once
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Renderer {
public:
    Renderer(int width, int height, const char* title);
    ~Renderer();
    void Clear();
    void Display();
    bool ShouldClose();
    bool IsKeyPressed(int key);
    unsigned int GetShaderProgram();
    void SetOffset(float x, float y);
private:
    GLFWwindow* window;
    unsigned int shaderProgram;
    int offsetLoc;
    void InitShaders();
};
