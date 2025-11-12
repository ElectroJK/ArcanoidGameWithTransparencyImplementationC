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
    glViewport(0, 0, width, height);
    glClearColor(0.02f, 0.12f, 0.45f, 1.0f);
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
        "#version 330 core\nlayout (location = 0) in vec2 pos;layout (location = 1) in vec2 uv;out vec2 vUV;uniform vec2 offset;void main(){vUV = uv;gl_Position = vec4(pos + offset,0.0,1.0);}";
    const char* fShader =
        "#version 330 core\nin vec2 vUV;out vec4 FragColor;uniform vec4 color;uniform bool useTex;uniform sampler2D tex;void main(){FragColor = useTex ? texture(tex, vUV) : color;}";
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
    colorLoc = glGetUniformLocation(shaderProgram, "color");
    useTexLoc = glGetUniformLocation(shaderProgram, "useTex");
    samplerLoc = glGetUniformLocation(shaderProgram, "tex");
}

unsigned int Renderer::GetShaderProgram() { return shaderProgram; }

void Renderer::SetOffset(float x, float y) { glUseProgram(shaderProgram); glUniform2f(offsetLoc, x, y); }
void Renderer::SetColor(float r, float g, float b, float a) { glUseProgram(shaderProgram); glUniform4f(colorLoc, r, g, b, a); }
void Renderer::UseTexture(bool enabled) { glUseProgram(shaderProgram); glUniform1i(useTexLoc, enabled ? 1 : 0); }
void Renderer::BindTexture(unsigned int texId) { glUseProgram(shaderProgram); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texId); glUniform1i(samplerLoc, 0); }

void Renderer::DrawQuad(float hw, float hh) {
    DrawQuadUV(hw, hh, 0.0f, 0.0f, 1.0f, 1.0f);
}

void Renderer::DrawQuadUV(float hw, float hh, float u0, float v0, float u1, float v1) {
    float verts[] = { -hw, -hh, u0, v0,  hw, -hh, u1, v0,  hw,  hh, u1, v1,  -hw,  hh, u0, v1 };
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

unsigned int Renderer::LoadTextureFromFile(const char* path) {
    using namespace Gdiplus;
    static bool started = false;
    static ULONG_PTR token;
    if (!started) { GdiplusStartupInput si; GdiplusStartup(&token, &si, nullptr); started = true; }
    wchar_t wpath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH);
    Bitmap* bmp = Bitmap::FromFile(wpath);
    if (!bmp || bmp->GetLastStatus() != Ok) { if (bmp) delete bmp; return 0; }
    BitmapData bd;
    Rect rect(0, 0, bmp->GetWidth(), bmp->GetHeight());
    bmp->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &bd);
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bd.Width, bd.Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bd.Scan0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    bmp->UnlockBits(&bd);
    delete bmp;
    return tex;
}

void Renderer::LoadGifFrames(const char* path, std::vector<unsigned int>& textures, std::vector<int>& delaysMs, int& width, int& height) {
    using namespace Gdiplus;
    static bool started = false;
    static ULONG_PTR token;
    if (!started) { GdiplusStartupInput si; GdiplusStartup(&token, &si, nullptr); started = true; }
    wchar_t wpath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH);
    std::unique_ptr<Bitmap> bmp(Bitmap::FromFile(wpath));
    if (!bmp || bmp->GetLastStatus() != Ok) return;
    GUID dimId;
    UINT count = bmp->GetFrameDimensionsCount();
    if (count == 0) return;
    std::vector<GUID> dims(count);
    bmp->GetFrameDimensionsList(dims.data(), count);
    UINT frameCount = bmp->GetFrameCount(&dims[0]);
    width = bmp->GetWidth();
    height = bmp->GetHeight();
    PropertyItem* pDelays = nullptr;
    UINT size = bmp->GetPropertyItemSize(PropertyTagFrameDelay);
    std::vector<int> frameDelays(frameCount, 100);
    if (size) {
        pDelays = (PropertyItem*)malloc(size);
        if (bmp->GetPropertyItem(PropertyTagFrameDelay, size, pDelays) == Ok) {
            UINT n = pDelays->length / 4;
            for (UINT i = 0; i < frameCount && i < n; ++i) frameDelays[i] = ((LONG*)pDelays->value)[i] * 10;
        }
        free(pDelays);
    }
    textures.clear();
    delaysMs.clear();
    for (UINT i = 0; i < frameCount; ++i) {
        bmp->SelectActiveFrame(&dims[0], i);
        BitmapData bd;
        Rect rect(0, 0, bmp->GetWidth(), bmp->GetHeight());
        bmp->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &bd);
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bd.Width, bd.Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bd.Scan0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        bmp->UnlockBits(&bd);
        textures.push_back(tex);
        delaysMs.push_back(frameDelays[i]);
    }
}
