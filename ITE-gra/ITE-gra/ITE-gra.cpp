#include "raylib.h"
#include "raymath.h"
#include "Enemy.h"
#include <vector>
#include <iostream>

struct Ghost {
    Vector2 pos;
    float angle;
    float alpha;
};

bool Paused = false;

int main() {
    InitWindow(800, 600, "ITE-gra");
    InitAudioDevice();
    SetTargetFPS(60);
    SetMasterVolume(1);

    Vector2 position = { 400, 300 };
    std::vector<Enemy> enemies;
    enemies.push_back(Enemy(Vector2{200, 200}, AttackType::CIRCLE, 1.5f, 1.0f, 3.0f, 60.0f, 50.0f, 250.0f));
    enemies.push_back(Enemy(Vector2{500, 300}, AttackType::IN_FRONT, 5.0f, 1.0f, 3.0f, 200.0f, 100.0f, 100.0f));

    const float SIZE = 20.0f;
    const float DASH_DISTANCE = 250.0f;
    const float DASH_DELAY = 0.13f;
    const float DASH_TIME = 0.13f;
    const float GHOST_LIFETIME = 0.35f;
    const int MAX_GHOSTS = 200;
    const float GHOST_SPACING = 8.0f;

    Ghost ghosts[MAX_GHOSTS] = {};
    int ghostIndex = 0;
    bool dashPending = false;
    float dashTimer = 0.0f;
    bool dashing = false;
    float dashT = 0.0f;
    Vector2 dashStart = { 0,0 };
    Vector2 dashEnd = { 0,0 };
    Vector2 dashDir = { 0,0 };
    float dashAngle = 0;
    float shakeTime = 0.0f;
    float shakeStrength = 1.0f;
    float accumulatedDistance = 0.0f;
    Vector2 lastGhostPos = { 0,0 };
    float anglePaused = 0.0f;
    int playerDist = 0;

    Sound dashSound = LoadSound("resources/sound/dzwiekDashWav.wav");

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();
        float angle = atan2f(mouse.y - position.y, mouse.x - position.x) * RAD2DEG;

        Vector2 shakeOffset = { 0, 0 };

        if (IsKeyPressed(KEY_SPACE)) {
            anglePaused = dashAngle;
            std::cout << anglePaused << std::endl;
            Paused = !Paused;
        }

        if (!Paused) {

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !dashPending && !dashing) {
                dashPending = true;
                dashTimer = DASH_DELAY;
                Vector2 dir = Vector2Subtract(mouse, position);
                float dist = Vector2Length(dir);
                if (dist > 0) dashDir = Vector2Normalize(dir);
                float dashLen = dist > DASH_DISTANCE ? DASH_DISTANCE : dist;
                dashStart = position;
                dashEnd = { position.x + dashDir.x * dashLen, position.y + dashDir.y * dashLen };
                dashAngle = angle;
                lastGhostPos = position;
                accumulatedDistance = 0.0f;
                PlaySound(dashSound);
            }

            if (dashPending) {
                dashTimer -= dt;
                if (dashTimer <= 0) {
                    dashPending = false;
                    dashing = true;
                    dashT = 0.0f;
                }
            }

            if (dashing) {
                dashT += dt / DASH_TIME;
                if (dashT > 1.0f) dashT = 1.0f;
                Vector2 prevPos = position;
                position = Vector2Lerp(dashStart, dashEnd, dashT);
                accumulatedDistance += Vector2Distance(prevPos, position);
                while (accumulatedDistance >= GHOST_SPACING) {
                    Vector2 spawnPos = Vector2Lerp(lastGhostPos, position, GHOST_SPACING / accumulatedDistance);
                    Ghost g;
                    g.pos = spawnPos;
                    g.angle = dashAngle;
                    g.alpha = 1.0f;
                    ghosts[ghostIndex] = g;
                    ghostIndex = (ghostIndex + 1) % MAX_GHOSTS;
                    accumulatedDistance -= GHOST_SPACING;
                    lastGhostPos = spawnPos;
                }
                if (dashT >= 1.0f) {
                    dashing = false;
                    shakeTime = 0.12f;
                }
            }

            for (int i = 0; i < MAX_GHOSTS; i++) {
                if (ghosts[i].alpha > 0.0f) ghosts[i].alpha -= dt / GHOST_LIFETIME;
                if (ghosts[i].alpha < 0.0f) ghosts[i].alpha = 0.0f;
            }


            if (shakeTime > 0) {
                shakeTime -= dt;
                shakeOffset.x = GetRandomValue(-shakeStrength, shakeStrength);
                shakeOffset.y = GetRandomValue(-shakeStrength, shakeStrength);
            }
           
            for (auto& e : enemies) {
                playerDist = sqrt(pow(position.x-e.position.x, 2) + pow(position.y-e.position.y, 2));
                e.Update(dt, position, playerDist);
            }

        }
        BeginDrawing();
        ClearBackground(RAYWHITE);

            BeginBlendMode(BLEND_ALPHA);
            for (int i = 0; i < MAX_GHOSTS; i++) {
                if (ghosts[i].alpha > 0.0f) {
                    Rectangle r = { ghosts[i].pos.x - SIZE / 2 + shakeOffset.x,
                                    ghosts[i].pos.y - SIZE / 2 + shakeOffset.y,
                                    SIZE, SIZE };
                    Color c = ColorAlpha(BLUE, ghosts[i].alpha * 0.5f);
                    DrawRectanglePro(r, { SIZE / 2, SIZE / 2 }, ghosts[i].angle, c);
                }
            }
            EndBlendMode();

            for (auto& e : enemies) e.Draw();

            Rectangle r = { position.x - SIZE / 2 + shakeOffset.x,
                            position.y - SIZE / 2 + shakeOffset.y,
                            SIZE, SIZE };

            DrawText("LPM - DASH", 10, 10, 20, DARKGRAY);

            if (!Paused) {
                DrawRectanglePro(r, { SIZE / 2, SIZE / 2 }, angle, BLUE);
            }else{
                DrawRectanglePro(r, { SIZE / 2, SIZE / 2 }, anglePaused, BLUE); // Ma zapamiętywać w jakim punkcie była myszka w momencie naciśnięcia pauzy.
                DrawRectanglePro({0, 0, 800.f, 600.f}, { 0, 0 }, 0.0f, ColorAlpha(BLACK, 0.5));
                DrawText("Paused!", 50, 50, 50, GREEN);
            }


        EndDrawing();
    }

    UnloadSound(dashSound);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
