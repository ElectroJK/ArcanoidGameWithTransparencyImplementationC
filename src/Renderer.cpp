#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include <iostream>

Renderer::Renderer(int width, int height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    InitShaders();
}

void Renderer::Clear() { glClear(GL_COLOR_BUFFER_BIT); }
void Renderer::Display() { glfwSwapBuffers(window); glfwPollEvents(); }
bool Renderer::ShouldClose() { return glfwWindowShouldClose(window); }
bool Renderer::IsKeyPressed(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }
Renderer::~Renderer() { glfwTerminate(); }

void Renderer::InitShaders() {
    const char* vShader =
        "#version 330 core\nlayout (location = 0) in vec2 pos;uniform vec2 offset;void main(){gl_Position = vec4(pos + offset,0.0,1.0);}";
    const char* fShader =
        "#version 330 core\nout vec4 FragColor;uniform vec4 color;void main(){FragColor = color;}";
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vShader, NULL);
    glCompileShader(vs);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fShader, NULL);
    glCompileShader(fs);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
    offsetLoc = glGetUniformLocation(shaderProgram, "offset");
}

unsigned int Renderer::GetShaderProgram() { return shaderProgram; }

void Renderer::SetOffset(float x, float y) {
    glUseProgram(shaderProgram);
    glUniform2f(offsetLoc, x, y);
}
