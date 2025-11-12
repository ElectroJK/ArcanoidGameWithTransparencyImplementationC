#pragma once
#include <vector>
#include <random>
#include "Renderer.h"
#include "Ball.h"
#include "Paddle.h"

class Game {
public:
    Game(int width, int height, const char* title);
    void Run();
private:
    struct Brick { float x, y, w, h; float r, g, b; int points; bool destroyed; };
    enum State { Serving, Playing, Win, GameOver };
    enum LevelType { LevelGrid, LevelAlien };

    Renderer renderer;
    Ball ball;
    Paddle paddle;
    std::vector<Brick> bricks;
    bool running;
    double lastTime;
    int lives;
    int score;
    State state;
    bool prevSpace;
    LevelType level;
    std::mt19937 rng;

    std::vector<unsigned int> danceFrames;
    std::vector<int> danceDelaysMs;
    int danceW, danceH;
    int danceFrame;
    double danceNextAt;
    bool showDance;
    double danceUntil;
    std::vector<unsigned int> sadFrames;
    std::vector<int> sadDelaysMs;
    int sadW, sadH;
    int sadFrame;
    double sadNextAt;
    bool showSad;
    std::vector<unsigned int> boomFrames;
    std::vector<int> boomDelaysMs;
    int boomW, boomH;
    int boomFrame;
    double boomNextAt;
    bool showBoom;
    float boomX, boomY;
    int boom2Frame;
    double boom2NextAt;
    bool showBoom2;
    float boom2X, boom2Y;
    std::vector<unsigned int> starFrames;
    std::vector<int> starDelaysMs;
    int starW, starH;
    struct Fx { float x, y; int frame; double nextAt; bool alive; };
    std::vector<Fx> brickFx;

    void ProcessInput();
    void InitLevel();
    void InitLevelAlien();
    void ResetBallOnPaddle();
    void UpdatePlaying(float dt);
    void DrawScene();
    void DrawHUD();
    bool AllBricksCleared() const;
    float hudTop;
    float bannerBottom;
};
