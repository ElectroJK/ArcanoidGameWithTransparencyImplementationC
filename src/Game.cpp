#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <windows.h>
#include <gdiplus.h>

struct M4 { float m[16]; };
static M4 I(){ M4 r={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}; return r; }
static M4 Mul(const M4&a,const M4&b){ M4 r={0}; for(int i=0;i<4;i++) for(int j=0;j<4;j++){ r.m[i*4+j]=a.m[i*4+0]*b.m[0*4+j]+a.m[i*4+1]*b.m[1*4+j]+a.m[i*4+2]*b.m[2*4+j]+a.m[i*4+3]*b.m[3*4+j]; } return r; }
static M4 T(float x,float y,float z){ M4 r=I(); r.m[12]=x; r.m[13]=y; r.m[14]=z; return r; }
static M4 Ry(float a){ float c=std::cos(a), s=std::sin(a); M4 r=I(); r.m[0]=c; r.m[2]=s; r.m[8]=-s; r.m[10]=c; return r; }
static M4 P(float fovy,float aspect,float zn,float zf){ float f=1.0f/std::tan(fovy*0.5f); M4 r={0}; r.m[0]=f/aspect; r.m[5]=f; r.m[10]=(zf+zn)/(zn-zf); r.m[11]=-1.0f; r.m[14]=(2*zf*zn)/(zn-zf); return r; }
static unsigned int Compile(unsigned int type,const char*src){ unsigned int s=glCreateShader(type); glShaderSource(s,1,&src,nullptr); glCompileShader(s); return s; }
static unsigned int Link(unsigned int vs,unsigned int fs){ unsigned int p=glCreateProgram(); glAttachShader(p,vs); glAttachShader(p,fs); glLinkProgram(p); glDeleteShader(vs); glDeleteShader(fs); return p; }
static unsigned int LoadTexGDI(const char* path){
    static bool started=false; static ULONG_PTR token; if(!started){ Gdiplus::GdiplusStartupInput si; GdiplusStartup(&token,&si,nullptr); started=true; }
    wchar_t wpath[MAX_PATH]; MultiByteToWideChar(CP_UTF8,0,path,-1,wpath,MAX_PATH);
    std::unique_ptr<Gdiplus::Bitmap> bmp{ Gdiplus::Bitmap::FromFile(wpath) };
    if(!bmp || bmp->GetLastStatus()!=Gdiplus::Ok) return 0;
    Gdiplus::BitmapData bd; Gdiplus::Rect rect(0,0,bmp->GetWidth(),bmp->GetHeight());
    bmp->LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&bd);
    GLuint tex=0; glGenTextures(1,&tex); glBindTexture(GL_TEXTURE_2D,tex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,bd.Width,bd.Height,0,GL_BGRA,GL_UNSIGNED_BYTE,bd.Scan0);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    bmp->UnlockBits(&bd);
    return tex;
}
static bool LoadSMF(const char* path, std::vector<float>& pos, std::vector<unsigned int>& idx){ std::ifstream in(path); if(!in) return false; std::string line; std::vector<float> verts; std::vector<unsigned int> faces; verts.reserve(10000); faces.reserve(20000); while(std::getline(in,line)){ if(line.empty()) continue; if(line[0]=='v' && (line.size()==1 || line[1]==' ')){ std::istringstream ss(line.substr(1)); float x,y,z; ss>>x>>y>>z; verts.push_back(x); verts.push_back(y); verts.push_back(z); } else if(line[0]=='f'){ std::istringstream ss(line.substr(1)); unsigned int a,b,c; ss>>a>>b>>c; if(a&&b&&c){ faces.push_back(a-1); faces.push_back(b-1); faces.push_back(c-1); } } }
 if(verts.empty()||faces.empty()) return false; pos.swap(verts); idx.swap(faces); return true; }
static void ShowThankWindow(GLFWwindow* returnWindow){ int w=1400,h=1200; glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3); glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE); GLFWwindow* win=glfwCreateWindow(w,h,"Thank You",nullptr,nullptr); if(!win) return; glfwMakeContextCurrent(win); gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); glViewport(0,0,w,h); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 const char* v2d = "#version 330 core\nlayout(location=0) in vec2 p; layout(location=1) in vec2 uv; out vec2 t; void main(){ t=uv; gl_Position=vec4(p,0.0,1.0);}";
 const char* f2d = "#version 330 core\nin vec2 t; out vec4 o; uniform sampler2D tex; void main(){ o = texture(tex,t); }";
 unsigned int vs2=Compile(GL_VERTEX_SHADER,v2d), fs2=Compile(GL_FRAGMENT_SHADER,f2d); unsigned int prog2=Link(vs2,fs2); int uTex=glGetUniformLocation(prog2,"tex"); unsigned int vao=0,vbo=0; glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
 using namespace Gdiplus; static bool started=false; static ULONG_PTR token; if(!started){ GdiplusStartupInput si; GdiplusStartup(&token,&si,nullptr); started=true; }
 int tyW=0, tyH=0; GLuint texTY=0; { wchar_t wpath[MAX_PATH]; MultiByteToWideChar(CP_UTF8,0,"src/fun/TY.png",-1,wpath,MAX_PATH); std::unique_ptr<Gdiplus::Bitmap> bmp{ Gdiplus::Bitmap::FromFile(wpath) }; if(bmp && bmp->GetLastStatus()==Gdiplus::Ok){ tyW=bmp->GetWidth(); tyH=bmp->GetHeight(); Gdiplus::BitmapData bd; Gdiplus::Rect rect(0,0,tyW,tyH); bmp->LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&bd); glGenTextures(1,&texTY); glBindTexture(GL_TEXTURE_2D,texTY); glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,bd.Width,bd.Height,0,GL_BGRA,GL_UNSIGNED_BYTE,bd.Scan0); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE); bmp->UnlockBits(&bd); } }
 std::vector<GLuint> gifFrames; std::vector<int> gifDelays; int gifW=0,gifH=0; { wchar_t wpath[MAX_PATH]; MultiByteToWideChar(CP_UTF8,0,"src/fun/oia.gif",-1,wpath,MAX_PATH); std::unique_ptr<Gdiplus::Bitmap> bmp{ Gdiplus::Bitmap::FromFile(wpath) }; if(bmp && bmp->GetLastStatus()==Gdiplus::Ok){ UINT count=bmp->GetFrameDimensionsCount(); if(count){ std::vector<GUID> dims(count); bmp->GetFrameDimensionsList(dims.data(),count); UINT frameCount=bmp->GetFrameCount(&dims[0]); gifW=bmp->GetWidth(); gifH=bmp->GetHeight(); Gdiplus::PropertyItem* pDelays=nullptr; UINT size=bmp->GetPropertyItemSize(PropertyTagFrameDelay); std::vector<int> frameDelays(frameCount,100); if(size){ pDelays=(Gdiplus::PropertyItem*)malloc(size); if(bmp->GetPropertyItem(PropertyTagFrameDelay,size,pDelays)==Gdiplus::Ok){ UINT n=pDelays->length/4; for(UINT i=0;i<frameCount && i<n;++i) frameDelays[i]=((LONG*)pDelays->value)[i]*10; } free(pDelays);} for(UINT i=0;i<frameCount;++i){ bmp->SelectActiveFrame(&dims[0],i); Gdiplus::BitmapData bd; Gdiplus::Rect rect(0,0,gifW,gifH); bmp->LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&bd); GLuint tex=0; glGenTextures(1,&tex); glBindTexture(GL_TEXTURE_2D,tex); glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,bd.Width,bd.Height,0,GL_BGRA,GL_UNSIGNED_BYTE,bd.Scan0); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE); bmp->UnlockBits(&bd); gifFrames.push_back(tex); gifDelays.push_back(frameDelays[i]); } } }
 int frame=0; double nextAt=0.0; while(!glfwWindowShouldClose(win)){
 glfwPollEvents(); int fbw, fbh; glfwGetFramebufferSize(win,&fbw,&fbh); glViewport(0,0,fbw,fbh); glClearColor(1.0f,1.0f,1.0f,1.0f); glClear(GL_COLOR_BUFFER_BIT);
 int topH = fbh>0 ? (fbh/2) : 0; int botH = fbh - topH; if(botH<1) botH=1; if(topH<1) topH=1;
 if(texTY && tyW>0 && tyH>0){ glUseProgram(prog2); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,texTY); glUniform1i(uTex,0); glViewport(0,botH,fbw,topH); float vpAspect=(fbw>0 && topH>0)?(float)fbw/(float)topH:1.0f; float imgAspect=(float)tyW/(float)tyH; float hh=1.0f; float hw=hh*(imgAspect/vpAspect); float verts[]={ -hw,-hh, 0,1,  hw,-hh, 1,1,  hw, hh, 1,0,  -hw, hh, 0,0 }; glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_DYNAMIC_DRAW); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0); glEnableVertexAttribArray(0); glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float))); glEnableVertexAttribArray(1); glDrawArrays(GL_TRIANGLE_FAN,0,4); }
 if(!gifFrames.empty() && gifW>0 && gifH>0){ double now=glfwGetTime(); if(now>=nextAt){ frame=(frame+1)%gifFrames.size(); int d=gifDelays.empty()?100:gifDelays[frame]; nextAt=now+((d<=0)?0.1:d/1000.0); } glUseProgram(prog2); glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,gifFrames[frame]); glUniform1i(uTex,0); glViewport(0,0,fbw,botH); float vpAspect=(fbw>0 && botH>0)?(float)fbw/(float)botH:1.0f; float imgAspect=(float)gifW/(float)gifH; float hh=1.0f; float hw=hh*(imgAspect/vpAspect); float verts[]={ -hw,-hh, 0,1,  hw,-hh, 1,1,  hw, hh, 1,0,  -hw, hh, 0,0 }; glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_DYNAMIC_DRAW); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0); glEnableVertexAttribArray(0); glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float))); glEnableVertexAttribArray(1); glDrawArrays(GL_TRIANGLE_FAN,0,4); }
 glfwSwapBuffers(win); if(glfwGetKey(win,GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(win,1); }
 if(texTY) glDeleteTextures(1,&texTY); for(auto t:gifFrames) glDeleteTextures(1,&t); glDeleteVertexArrays(1,&vao); glDeleteBuffers(1,&vbo); glDeleteProgram(prog2); glfwMakeContextCurrent(returnWindow); glfwDestroyWindow(win); }
}


void Game::DrawHUD() {
    float topY = hudTop;
    renderer.UseTexture(false);

    auto drawChar3x5 = [&](float x, char ch){
        unsigned char map[5] = {0};
        switch(ch){
            case 'S': map[0]=0b111; map[1]=0b100; map[2]=0b111; map[3]=0b001; map[4]=0b111; break;
            case 'C': map[0]=0b111; map[1]=0b100; map[2]=0b100; map[3]=0b100; map[4]=0b111; break;
            case 'O': map[0]=0b111; map[1]=0b101; map[2]=0b101; map[3]=0b101; map[4]=0b111; break;
            case 'R': map[0]=0b110; map[1]=0b101; map[2]=0b110; map[3]=0b101; map[4]=0b101; break;
            case 'E': map[0]=0b111; map[1]=0b100; map[2]=0b110; map[3]=0b100; map[4]=0b111; break;
            case ':': map[0]=0b000; map[1]=0b010; map[2]=0b000; map[3]=0b010; map[4]=0b000; break;
            default:  map[0]=map[1]=map[2]=map[3]=map[4]=0; break;
        }
        float cell = 0.008f;
        float startY = topY + 0.002f;
        for(int r=0;r<5;++r){
            for(int c=0;c<3;++c){
                if (map[r] & (1<<(2-c))){
                    renderer.SetColor(1,1,0,1);
                    renderer.SetOffset(x + (c*cell*2), startY - r*cell*2);
                    renderer.DrawQuad(cell, cell);
                }
            }
        }
        return x + cell*3*2 + 0.012f;
    };

    float x = -0.92f;
    const char* label = "SCORE:";
    for (const char* p = label; *p; ++p) x = drawChar3x5(x, *p);

    auto drawDigit3x5 = [&](float dx, int d){
        unsigned char rows[5]={0};
        switch(d){
            case 0: rows[0]=0b111; rows[1]=0b101; rows[2]=0b101; rows[3]=0b101; rows[4]=0b111; break;
            case 1: rows[0]=0b010; rows[1]=0b110; rows[2]=0b010; rows[3]=0b010; rows[4]=0b111; break;
            case 2: rows[0]=0b111; rows[1]=0b001; rows[2]=0b111; rows[3]=0b100; rows[4]=0b111; break;
            case 3: rows[0]=0b111; rows[1]=0b001; rows[2]=0b111; rows[3]=0b001; rows[4]=0b111; break;
            case 4: rows[0]=0b101; rows[1]=0b101; rows[2]=0b111; rows[3]=0b001; rows[4]=0b001; break;
            case 5: rows[0]=0b111; rows[1]=0b100; rows[2]=0b111; rows[3]=0b001; rows[4]=0b111; break;
            case 6: rows[0]=0b111; rows[1]=0b100; rows[2]=0b111; rows[3]=0b101; rows[4]=0b111; break;
            case 7: rows[0]=0b111; rows[1]=0b001; rows[2]=0b001; rows[3]=0b001; rows[4]=0b001; break;
            case 8: rows[0]=0b111; rows[1]=0b101; rows[2]=0b111; rows[3]=0b101; rows[4]=0b111; break;
            case 9: rows[0]=0b111; rows[1]=0b101; rows[2]=0b111; rows[3]=0b001; rows[4]=0b111; break;
            default: rows[0]=rows[1]=rows[2]=rows[3]=rows[4]=0; break;
        }
        float cell=0.008f;
        float startY = topY + 0.002f;
        for(int r=0;r<5;++r){
            for(int c=0;c<3;++c){
                if (rows[r] & (1<<(2-c))){
                    renderer.SetColor(1,1,0,1);
                    renderer.SetOffset(dx + (c*cell*2), startY - r*cell*2);
                    renderer.DrawQuad(cell, cell);
                }
            }
        }
        return dx + cell*3*2 + 0.012f;
    };

    int s = score;
    float sx = x + 0.02f;
    if (s == 0) {
        sx = drawDigit3x5(sx, 0);
    } else {
        int digits[12]; int n=0; while(s>0 && n<12){ digits[n++]=s%10; s/=10; }
        for(int i=n-1;i>=0;--i){ sx = drawDigit3x5(sx, digits[i]); }
    }

    for (int i=0;i<lives;i++) {
        renderer.UseTexture(false);
        renderer.SetColor(1.0f, 0.3f, 0.3f, 1.0f);
        renderer.SetOffset(0.0f, 0.0f);
        float cx = 0.8f + i*0.06f;
        float cy = hudTop;
        float r = 0.02f;
        const int N = 24;
        std::vector<float> verts; verts.reserve((N+2)*2);
        verts.push_back(cx); verts.push_back(cy);
        for(int k=0;k<=N;k++){
            float ang = (float)k / (float)N * 6.28318530718f;
            verts.push_back(cx + r * std::cos(ang));
            verts.push_back(cy + r * std::sin(ang));
        }
        unsigned int vao=0, vbo=0;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), verts.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, N+2);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
}

Game::Game(int width, int height, const char* title)
    : renderer(width, height, title), ball(0.0f, 0.0f, 0.05f), paddle(0.0f, -0.60f, 0.2f, 0.05f) {
    running = true;
    lastTime = glfwGetTime();
    lives = 3;
    score = 0;
    state = Serving;
    level = LevelGrid;
    prevSpace = false;
    hudTop = 0.95f;
    bannerBottom = -0.95f;
    renderer.LoadGifFrames("src/fun/ttdance.gif", danceFrames, danceDelaysMs, danceW, danceH);
    renderer.LoadGifFrames("src/fun/sadcat.gif", sadFrames, sadDelaysMs, sadW, sadH);
    renderer.LoadGifFrames("src/fun/BOOM.gif", boomFrames, boomDelaysMs, boomW, boomH);
    renderer.LoadGifFrames("src/fun/brick_exlode.gif", starFrames, starDelaysMs, starW, starH);
    danceFrame = 0; danceNextAt = 0.0; showDance = false; danceUntil = 0.0;
    sadFrame = 0; sadNextAt = 0.0; showSad = false;
    boomFrame = 0; boomNextAt = 0.0; showBoom = false; boomX = 0.0f; boomY = 0.0f;
    boom2Frame = 0; boom2NextAt = 0.0; showBoom2 = false; boom2X = 0.0f; boom2Y = 0.0f;
    brickFx.clear();
    std::random_device rd; rng.seed(rd());
    basePaddleW = paddle.GetW();
    tripleUntil = 0.0; enlargeUntil = 0.0;
    powerUps.clear(); extraBalls.clear();
    InitLevel();
    ResetBallOnPaddle();
}

void Game::InitLevel() {
    bricks.clear();
    int cols = 12;
    int rows = 6;
    float left = -0.94f;
    float right = 0.94f;
    float playTop = 0.9f;
    float top = playTop - 0.2f;
    float bw_full = (right - left) / cols;
    float bh_full = 0.08f;
    float bw = bw_full * 0.5f * 0.92f;
    float bh = bh_full * 0.5f * 0.8f;
    float y = top;
    float colors[6][3] = {
        {0.6f, 0.6f, 0.6f},
        {1.0f, 0.5f, 0.0f},
        {1.0f, 0.9f, 0.2f},
        {0.2f, 0.9f, 0.2f},
        {0.2f, 0.8f, 1.0f},
        {0.9f, 0.2f, 0.9f}
    };
    int pointsRow[6] = {120, 100, 80, 70, 60, 50};
    for (int r = 0; r < rows; ++r) {
        float x = left + bw_full * 0.5f;
        for (int c = 0; c < cols; ++c) {
            Brick b; b.x = x; b.y = y; b.w = bw; b.h = bh; b.r = colors[r][0]; b.g = colors[r][1]; b.b = colors[r][2]; b.points = pointsRow[r]; b.destroyed = false;
            bricks.push_back(b);
            x += bw_full;
        }
        y -= bh_full;
    }
}

void Game::InitLevelAlien() {
    bricks.clear();
    const char* art[] = {
        "......yy........yy......",
        "......yy........yy......",
        "....gggggggggggggggg....",
        "...gggggggggggggggggg...",
        "..gggggrrgggggggrrgggg..",
        "..gggggggggggggggggggg..",
        "..gggggggggggggggggggg..",
        "gggggggggggggggggggggggg",
        "gggggggggggggggggggggggg",
        "..gggggggg....gggggggg..",
        "..gggggg........gggggg..",
        "....gg............gg....",
        "....gg............gg....",
        "......gg........gg......"
    };
    int rows = sizeof(art)/sizeof(art[0]);
    int cols = 24;

    float left = -0.94f;
    float right = 0.94f;
    float playTop = 0.9f;
    float top = playTop - 0.08f;
    float cellW = (right - left) / cols;
    float cellH = 0.06f;
    float bw = cellW * 0.46f;
    float bh = cellH * 0.4f;

    float y = top;
    for (int r = 0; r < rows; ++r) {
        float x = left + cellW * 0.5f;
        for (int c = 0; c < cols; ++c) {
            char ch = art[r][c];
            if (ch == '.') { x += cellW; continue; }
            Brick b; b.x = x; b.y = y; b.w = bw; b.h = bh; b.destroyed = false;
            if (ch == 'r') { b.r=1.0f; b.g=0.2f; b.b=0.2f; b.points=250; }
            else { b.r=1.0f; b.g=1.0f; b.b=1.0f; b.points= (ch=='y'?200:150); }
            bricks.push_back(b);
            x += cellW;
        }
        y -= cellH;
    }
}

void Game::ResetBallOnPaddle() {
    ball.SetX(paddle.GetX());
    ball.SetY(paddle.GetY() + paddle.GetH() + ball.GetRadius() + 0.002f);
    ball.SetVX(0.0f);
    ball.SetVY(0.0f);
}

bool Game::AllBricksCleared() const {
    for (auto& b : bricks) if (!b.destroyed) return false;
    return true;
}

void Game::UpdatePlaying(float dt) {
    bool mainFell = false;
    float lastFallX = ball.GetX();
    float lastFallY = ball.GetY();

    auto processBall = [&](Ball& bb, bool isMain){
        bb.Update(dt);
        float bx = bb.GetX();
        float by = bb.GetY();
        float br = bb.GetRadius();
        float playTop = 0.9f;
        float playBottom = paddle.GetY() - 0.08f;
        if (by + br > playTop) { bb.SetY(playTop - br); bb.SetVY(-bb.GetVY()); }
        if (by - br < playBottom) {
            if (isMain) mainFell = true;
            lastFallX = bx; lastFallY = by;
            return false;
        }
        float px = paddle.GetX(); float py = paddle.GetY(); float pw = paddle.GetW(); float ph = paddle.GetH();
        if (bb.GetVY() < 0.0f && by > py && by - br <= py + ph + 0.02f && bx >= px - pw - br && bx <= px + pw + br) {
            float hitPos = (bx - px) / pw;
            if (hitPos < -1.0f) hitPos = -1.0f;
            if (hitPos > 1.0f) hitPos = 1.0f;
            float speed = std::sqrt(bb.GetVX()*bb.GetVX() + bb.GetVY()*bb.GetVY());
            if (speed < 0.6f) speed = 0.9f;
            float minDeg = 25.0f * 3.1415926535f / 180.0f;
            float maxDeg = 155.0f * 3.1415926535f / 180.0f;
            float t = (hitPos + 1.0f) * 0.5f;
            float theta = minDeg + t * (maxDeg - minDeg);
            bb.SetVX(speed * std::cos(theta));
            bb.SetVY(speed * std::sin(theta));
            bb.SetY(py + ph + br + 0.003f);
        }
        for (auto& b : bricks) {
            if (b.destroyed) continue;
            float rcx = bx; if (bx < b.x - b.w) rcx = b.x - b.w; else if (bx > b.x + b.w) rcx = b.x + b.w;
            float rcy = by; if (by < b.y - b.h) rcy = b.y - b.h; else if (by > b.y + b.h) rcy = b.y + b.h;
            float rdx = bx - rcx; float rdy = by - rcy;
            if (rdx*rdx + rdy*rdy <= br*br) {
                b.destroyed = true; score += 5;
                if (std::fabs(rdx) > std::fabs(rdy)) bb.SetVX(-bb.GetVX()); else bb.SetVY(-bb.GetVY());
                if (!danceFrames.empty()) { showDance = true; danceFrame = 0; double now = glfwGetTime(); danceNextAt = now + (danceDelaysMs.empty()?0.1: danceDelaysMs[0]/1000.0); danceUntil = now + 3.0; }
                if (!starFrames.empty()) { Fx fx; fx.x = b.x; fx.y = b.y; fx.frame = 0; fx.nextAt = glfwGetTime() + (starDelaysMs.empty()?0.08: std::max(10, starDelaysMs[0])/1000.0); fx.alive = true; brickFx.push_back(fx); }
                std::uniform_int_distribution<int> roll(0,99);
                int v = roll(rng);
                if (v < 10) { PowerUp pu; pu.x = b.x; pu.y = b.y; pu.vy = -0.25f; pu.type = (v < 5 ? 0 : 1); pu.alive = true; powerUps.push_back(pu); }
                break;
            }
        }
        return true;
    };

    bool mainAlive = processBall(ball, true);

    for (size_t i=0; i<extraBalls.size();) {
        if (!processBall(extraBalls[i], false)) {
            extraBalls.erase(extraBalls.begin() + i);
        } else {
            ++i;
        }
    }

    if (!mainAlive && !extraBalls.empty()) {
        ball = extraBalls.front();
        extraBalls.erase(extraBalls.begin());
        mainAlive = true;
    }

    if (!mainAlive && extraBalls.empty()) {
        lives -= 1;
        showDance = false; danceUntil = 0.0;
        double n = glfwGetTime();
        if (lives > 0) {
            state = Serving;
            extraBalls.clear(); powerUps.clear(); tripleUntil = 0.0; enlargeUntil = 0.0; paddle.SetW(basePaddleW);
            ResetBallOnPaddle();
        } else {
            state = GameOver;
            showSad = true; showDance = false; sadFrame = 0; sadNextAt = n + (sadDelaysMs.empty()?0.1: sadDelaysMs[0]/1000.0);
            showBoom = true; boomFrame = 0; boomX = paddle.GetX(); boomY = paddle.GetY(); boomNextAt = n + 0.08;
            showBoom2 = true; boom2Frame = 0; boom2X = lastFallX; boom2Y = lastFallY; boom2NextAt = n + 0.08;
        }
        return;
    }

    double now = glfwGetTime();
    if (tripleUntil>0.0 && now>tripleUntil) { extraBalls.clear(); tripleUntil=0.0; }
    if (enlargeUntil>0.0 && now>enlargeUntil) { paddle.SetW(basePaddleW); enlargeUntil=0.0; }

    for (auto& pu : powerUps) {
        if (!pu.alive) continue;
        pu.y += pu.vy * dt;
        if (pu.y < -1.1f) { pu.alive = false; continue; }
        float px = paddle.GetX(); float py = paddle.GetY(); float pw = paddle.GetW(); float ph = paddle.GetH();
        if (pu.y <= py + ph*0.5f && pu.y >= py - ph*0.5f && pu.x >= px - pw && pu.x <= px + pw) {
            pu.alive = false;
            if (pu.type==0) {
                Ball b1(ball.GetX(), ball.GetY(), ball.GetRadius());
                Ball b2(ball.GetX(), ball.GetY(), ball.GetRadius());
                b1.SetVX(ball.GetVX()*0.9f); b1.SetVY(ball.GetVY()*1.05f);
                b2.SetVX(ball.GetVX()*1.05f); b2.SetVY(ball.GetVY()*0.9f);
                extraBalls.clear(); extraBalls.push_back(b1); extraBalls.push_back(b2);
                tripleUntil = now + 20.0;
            } else {
                paddle.SetW(basePaddleW * 1.6f);
                enlargeUntil = now + 20.0;
            }
        }
    }
    powerUps.erase(std::remove_if(powerUps.begin(), powerUps.end(), [](const PowerUp& p){return !p.alive;}), powerUps.end());

    if (AllBricksCleared()) { state = Win; showSad = false; showDance = true; if (!danceFrames.empty()) { danceFrame = 0; double now2 = glfwGetTime(); danceNextAt = now2 + (danceDelaysMs.empty()?0.1: danceDelaysMs[0]/1000.0); } }
}

void Game::DrawScene() {
    for (auto& b : bricks) {
        if (b.destroyed) continue;
        renderer.SetColor(b.r, b.g, b.b, 1.0f);
        renderer.SetOffset(b.x, b.y);
        renderer.DrawQuad(b.w, b.h);
    }
    if (!(state == GameOver)) {
        paddle.Draw(renderer);
        ball.Draw(renderer);
        for (auto& eb : extraBalls) eb.Draw(renderer);
        for (auto& pu : powerUps) {
            renderer.UseTexture(false);
            if (pu.type==0) renderer.SetColor(0.2f, 0.6f, 1.0f, 1.0f); else renderer.SetColor(0.2f, 1.0f, 0.4f, 1.0f);
            renderer.SetOffset(pu.x, pu.y);
            renderer.DrawQuad(0.03f, 0.03f);
        }
    }
    DrawHUD();
    double now = glfwGetTime();
    for (auto& fx : brickFx) {
        if (!fx.alive) continue;
        if (now >= fx.nextAt) {
            fx.frame = fx.frame + 1;
            double delay = starDelaysMs.empty()?80.0: starDelaysMs[std::min((int)starDelaysMs.size()-1, fx.frame)];
            fx.nextAt = now + (delay <= 0 ? 0.08 : delay/1000.0);
        }
        if (fx.frame >= (int)starFrames.size()) { fx.alive = false; continue; }
        renderer.UseTexture(true);
        renderer.BindTexture(starFrames[fx.frame]);
        float hhx = 0.04f;
        float aspectx = (starW > 0 && starH > 0) ? ((float)starW / (float)starH) : 1.0f;
        float hwx = hhx * aspectx;
        renderer.SetOffset(fx.x, fx.y);
        renderer.DrawQuadUV(hwx, hhx, 0.0f, 1.0f, 1.0f, 0.0f);
        renderer.UseTexture(false);
    }
    brickFx.erase(std::remove_if(brickFx.begin(), brickFx.end(), [](const Fx& e){return !e.alive;}), brickFx.end());
    if (state == GameOver && (showBoom || showBoom2) && !boomFrames.empty()) {
        if (showBoom) {
            if (now >= boomNextAt) { boomFrame = (boomFrame + 1); double delayb = boomDelaysMs.empty()?90.0: boomDelaysMs[std::min((int)boomDelaysMs.size()-1, boomFrame)]; boomNextAt = now + (delayb <= 0 ? 0.09 : delayb/1000.0); }
            int idx = std::min(boomFrame, (int)boomFrames.size()-1);
            renderer.UseTexture(true);
            renderer.BindTexture(boomFrames[idx]);
            float hhb = 0.12f;
            float aspectb = (boomW > 0 && boomH > 0) ? ((float)boomW / (float)boomH) : 1.0f;
            float hwb = hhb * aspectb;
            renderer.SetOffset(boomX, boomY);
            renderer.DrawQuadUV(hwb, hhb, 0.0f, 1.0f, 1.0f, 0.0f);
            renderer.UseTexture(false);
            if (boomFrame >= (int)boomFrames.size()-1) showBoom = false;
        }
        if (showBoom2) {
            if (now >= boom2NextAt) { boom2Frame = (boom2Frame + 1); double delayb2 = boomDelaysMs.empty()?90.0: boomDelaysMs[std::min((int)boomDelaysMs.size()-1, boom2Frame)]; boom2NextAt = now + (delayb2 <= 0 ? 0.09 : delayb2/1000.0); }
            int idx2 = std::min(boom2Frame, (int)boomFrames.size()-1);
            renderer.UseTexture(true);
            renderer.BindTexture(boomFrames[idx2]);
            float hhb2 = 0.10f;
            float aspectb2 = (boomW > 0 && boomH > 0) ? ((float)boomW / (float)boomH) : 1.0f;
            float hwb2 = hhb2 * aspectb2;
            renderer.SetOffset(boom2X, boom2Y);
            renderer.DrawQuadUV(hwb2, hhb2, 0.0f, 1.0f, 1.0f, 0.0f);
            renderer.UseTexture(false);
            if (boom2Frame >= (int)boomFrames.size()-1) showBoom2 = false;
        }
    }
    if (state == GameOver && showSad && !sadFrames.empty()) {
        if (now >= sadNextAt) {
            sadFrame = (sadFrame + 1) % sadFrames.size();
            double delay = sadDelaysMs.empty()?100.0: sadDelaysMs[sadFrame];
            sadNextAt = now + (delay <= 0 ? 0.1 : delay/1000.0);
        }
        renderer.UseTexture(true);
        renderer.BindTexture(sadFrames[sadFrame]);
        float allowedTop2 = paddle.GetY() - 0.06f;
        float bottomLimit2 = -0.98f;
        float availH2 = std::max(0.02f, allowedTop2 - bottomLimit2);
        float hh2 = availH2 * 0.5f;
        float aspect2 = (sadW > 0 && sadH > 0) ? ((float)sadW / (float)sadH) : 16.0f/9.0f;
        float hw2 = hh2 * aspect2;
        float centerY2 = bottomLimit2 + hh2;
        if (centerY2 + hh2 > allowedTop2) { centerY2 = (allowedTop2 + bottomLimit2) * 0.5f; }
        renderer.SetOffset(0.0f, centerY2);
        renderer.DrawQuadUV(hw2, hh2, 0.0f, 1.0f, 1.0f, 0.0f);
        renderer.UseTexture(false);
    } else if ((!danceFrames.empty()) && (state == Win || (showDance && now <= danceUntil))) {
        if (now >= danceNextAt) {
            danceFrame = (danceFrame + 1) % danceFrames.size();
            double delay = danceDelaysMs.empty()?100.0: danceDelaysMs[danceFrame];
            danceNextAt = now + (delay <= 0 ? 0.1 : delay/1000.0);
        }
        renderer.UseTexture(true);
        renderer.BindTexture(danceFrames[danceFrame]);
        float allowedTop = paddle.GetY() - 0.06f;
        float bottomLimit = -0.98f;
        float availH = std::max(0.02f, allowedTop - bottomLimit);
        float hh = availH * 0.5f;
        float aspect = (danceW > 0 && danceH > 0) ? ((float)danceW / (float)danceH) : 16.0f/9.0f;
        float hw = hh * aspect;
        float centerY = bottomLimit + hh;
        if (centerY + hh > allowedTop) { centerY = (allowedTop + bottomLimit) * 0.5f; }
        renderer.SetOffset(0.0f, centerY);
        renderer.DrawQuadUV(hw, hh, 0.0f, 1.0f, 1.0f, 0.0f);
        renderer.UseTexture(false);
        if (showDance && now > danceUntil && state != Win) showDance = false;
    }
}

void Game::Run() {
    while (running && !renderer.ShouldClose()) {
        double now = glfwGetTime();
        float dt = static_cast<float>(now - lastTime);
        lastTime = now;
        ProcessInput();
        renderer.Clear();
        if (state == Playing) UpdatePlaying(dt);
        if (state == Serving) { showSad = false; }
        DrawScene();
        renderer.Display();
    }
}

void Game::ProcessInput() {
    static double prev = glfwGetTime();
    double now = glfwGetTime();
    float dt = static_cast<float>(now - prev);
    prev = now;
    if (renderer.IsKeyPressed(GLFW_KEY_ESCAPE)) running = false;
    bool left = renderer.IsKeyPressed(GLFW_KEY_LEFT) || renderer.IsKeyPressed(GLFW_KEY_A);
    bool right = renderer.IsKeyPressed(GLFW_KEY_RIGHT) || renderer.IsKeyPressed(GLFW_KEY_D);
    if (left || right) paddle.Update(left, right, dt);
    if (state == Serving) {
        ball.SetX(paddle.GetX());
        ball.SetY(paddle.GetY() + paddle.GetH() + ball.GetRadius() + 0.002f);
        ball.SetVX(0.0f);
        ball.SetVY(0.0f);
    }
    static bool prevH = false;
    bool h = renderer.IsKeyPressed(GLFW_KEY_H);
    if (h && !prevH) {
        if (level == LevelGrid) level = LevelAlien; else level = LevelGrid;
        if (level == LevelGrid) InitLevel(); else InitLevelAlien();
        lives = 3;
        score = 0;
        showSad = false; showDance = false; danceUntil = 0.0;
        showBoom = false; boomFrame = 0; showBoom2 = false; boom2Frame = 0; brickFx.clear();
        state = Serving;
        ResetBallOnPaddle();
    }
    prevH = h;
    static bool prevO = false;
    bool o = renderer.IsKeyPressed(GLFW_KEY_O);
    if (o && !prevO) {
        ShowThankWindow(renderer.GetWindow());
    }
    prevO = o;
    bool space = renderer.IsKeyPressed(GLFW_KEY_SPACE);
    if (state == Serving && space && !prevSpace) {
        ball.SetVX(0.0f);
        ball.SetVY(0.9f);
        state = Playing;
    }
    if ((state == GameOver || state == Win) && space && !prevSpace) {
        lives = 3; score = 0; if (level == LevelGrid) InitLevel(); else InitLevelAlien(); showSad = false; showDance = false; danceUntil = 0.0; showBoom = false; boomFrame = 0; showBoom2 = false; boom2Frame = 0; brickFx.clear(); state = Serving; ResetBallOnPaddle();
    }
    prevSpace = space;
}