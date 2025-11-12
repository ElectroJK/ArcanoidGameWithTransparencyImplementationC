#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include <cmath>
#include <algorithm>

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
        renderer.SetColor(1.0f, 0.3f, 0.3f, 1.0f);
        renderer.SetOffset(0.8f + i*0.06f, hudTop);
        renderer.DrawQuad(0.02f, 0.02f);
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
        {1.0f, 0.2f, 0.2f},
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
            if (ch == 'g') { b.r=0.75f; b.g=0.75f; b.b=0.75f; b.points=150; }
            else if (ch == 'y') { b.r=1.0f; b.g=0.9f; b.b=0.1f; b.points=200; }
            else if (ch == 'r') { b.r=0.9f; b.g=0.1f; b.b=0.1f; b.points=250; }
            else { b.r=0.6f; b.g=0.6f; b.b=0.6f; b.points=100; }
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
    ball.Update(dt);
    float bx = ball.GetX();
    float by = ball.GetY();
    float br = ball.GetRadius();
    float playTop = 0.9f;
    float playBottom = paddle.GetY() - 0.08f;
    if (by + br > playTop) { ball.SetY(playTop - br); ball.SetVY(-ball.GetVY()); }
    if (by - br < playBottom) {
        lives -= 1;
        showDance = false;
        danceUntil = 0.0;
        if (lives > 0) { state = Serving; ResetBallOnPaddle(); }
        else { state = GameOver; showSad = true; showDance = false; sadFrame = 0; double n = glfwGetTime(); sadNextAt = n + (sadDelaysMs.empty()?0.1: sadDelaysMs[0]/1000.0); showBoom = true; boomFrame = 0; boomX = paddle.GetX(); boomY = paddle.GetY(); boomNextAt = n + 0.08; showBoom2 = true; boom2Frame = 0; boom2X = bx; boom2Y = by; boom2NextAt = n + 0.08; }
        return;
    }
    float px = paddle.GetX();
    float py = paddle.GetY();
    float pw = paddle.GetW();
    float ph = paddle.GetH();
    if (ball.GetVY() < 0.0f && by > py && by - br <= py + ph + 0.02f && bx >= px - pw - br && bx <= px + pw + br) {
        float hitPos = (bx - px) / pw;
        if (hitPos < -1.0f) hitPos = -1.0f;
        if (hitPos > 1.0f) hitPos = 1.0f;
        float speed = std::sqrt(ball.GetVX()*ball.GetVX() + ball.GetVY()*ball.GetVY());
        if (speed < 0.6f) speed = 0.9f;
        float minDeg = 25.0f * 3.1415926535f / 180.0f;
        float maxDeg = 155.0f * 3.1415926535f / 180.0f;
        float t = (hitPos + 1.0f) * 0.5f;
        float theta = minDeg + t * (maxDeg - minDeg);
        ball.SetVX(speed * std::cos(theta));
        ball.SetVY(speed * std::sin(theta));
        ball.SetY(py + ph + br + 0.003f);
    }
    for (auto& b : bricks) {
        if (b.destroyed) continue;
        float rcx = bx; if (bx < b.x - b.w) rcx = b.x - b.w; else if (bx > b.x + b.w) rcx = b.x + b.w;
        float rcy = by; if (by < b.y - b.h) rcy = b.y - b.h; else if (by > b.y + b.h) rcy = b.y + b.h;
        float rdx = bx - rcx; float rdy = by - rcy;
        if (rdx*rdx + rdy*rdy <= br*br) {
            b.destroyed = true;
            score += 5;
            if (std::fabs(rdx) > std::fabs(rdy)) ball.SetVX(-ball.GetVX()); else ball.SetVY(-ball.GetVY());
            if (!danceFrames.empty()) { showDance = true; danceFrame = 0; double now = glfwGetTime(); danceNextAt = now + (danceDelaysMs.empty()?0.1: danceDelaysMs[0]/1000.0); danceUntil = now + 3.0; }
            if (!starFrames.empty()) { Fx fx; fx.x = b.x; fx.y = b.y; fx.frame = 0; fx.nextAt = glfwGetTime() + (starDelaysMs.empty()?0.08: std::max(10, starDelaysMs[0])/1000.0); fx.alive = true; brickFx.push_back(fx); }
            break;
        }
    }
    if (AllBricksCleared()) { state = Win; showSad = false; showDance = true; if (!danceFrames.empty()) { danceFrame = 0; double now2 = glfwGetTime(); danceNextAt = now2 + (danceDelaysMs.empty()?0.1: danceDelaysMs[0]/1000.0); } }
}

void Game::DrawScene() {
    for (auto& b : bricks) {
        if (b.destroyed) continue;
        renderer.UseTexture(false);
        renderer.SetColor(b.r, b.g, b.b, 1.0f);
        renderer.SetOffset(b.x, b.y);
        renderer.DrawQuad(b.w, b.h);
    }
    if (!(state == GameOver)) {
        paddle.Draw(renderer);
        ball.Draw(renderer);
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
