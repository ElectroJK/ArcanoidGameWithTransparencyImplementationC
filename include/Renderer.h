#pragma once
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

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
    void SetColor(float r, float g, float b, float a);
    void UseTexture(bool enabled);
    void BindTexture(unsigned int texId);
    void DrawQuad(float hw, float hh);
    void DrawQuadUV(float hw, float hh, float u0, float v0, float u1, float v1);
    unsigned int LoadTextureFromFile(const char* path);
    void LoadGifFrames(const char* path, std::vector<unsigned int>& textures, std::vector<int>& delaysMs, int& width, int& height);
private:
    GLFWwindow* window;
    unsigned int shaderProgram;
    int offsetLoc;
    int colorLoc;
    int useTexLoc;
    int samplerLoc;
    void InitShaders();
};
