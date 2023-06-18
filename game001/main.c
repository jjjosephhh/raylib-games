#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

struct Target
{
    Vector2 pos;
    Vector2 speed;
    float radius;
};

int generateRandomNumber(int min, int max)
{
    srand(time(0));
    return (rand() % (max - min + 1)) + min;
}

void updateTargetPosition(struct Target *target, int screenWidth, int screenHeight)
{
    target->pos.x += target->speed.x;
    target->pos.y += target->speed.y;

    if (target->pos.x + target->radius >= screenWidth)
    {
        target->pos.x = screenWidth - target->radius - 1;
        target->speed.x *= -1 * (float)generateRandomNumber(80, 120) / 100.0;
    }

    if (target->pos.y + target->radius >= screenHeight)
    {
        target->pos.y = screenHeight - target->radius - 1;
        target->speed.y *= -1 * (float)generateRandomNumber(80, 120) / 100.0;
    }

    if (target->pos.x - target->radius <= 0)
    {
        target->pos.x = target->radius + 1;
        target->speed.x *= -1 * (float)generateRandomNumber(80, 120) / 100.0;
    }

    if (target->pos.y - target->radius <= 0)
    {
        target->pos.y = target->radius + 1;
        target->speed.y *= -1 * (float)generateRandomNumber(80, 120) / 100.0;
    }
}

double calcEucDist(Vector2 *v1, Vector2 *v2)
{
    float dx = v2->x - v1->x;
    float dy = v2->y - v1->y;
    return sqrt(dx * dx + dy * dy);
}

enum GameState
{
    MENU,
    PLAY
};

int main(void)
{

    const int screenWidth = 850;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Shooting Gallery");
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    int textVertOffset = 20;
    int score = 0;
    char scoreMessage[20];
    int timeLimit = 15;
    int timeBase = 0;
    int timer = 0;
    char timerMessage[30];

    struct Color color1;
    color1.r = 255;
    color1.g = 100;
    color1.b = 100;
    color1.a = 255;

    struct Color color2;
    color2.r = 100;
    color2.g = 200;
    color2.b = 56;
    color2.a = 255;

    struct Color color3;
    color3.r = 19;
    color3.g = 35;
    color3.b = 210;
    color3.a = 255;

    InitAudioDevice();
    Sound popSound = LoadSound("resources/pop.wav");

    Texture2D crosshairs = LoadTexture("resources/icons8-target-48.png");

    Image targetImage = LoadImage("resources/badrobot.png");
    ImageResizeNN(&targetImage, 100, 100);
    Texture2D targetTexture = LoadTextureFromImage(targetImage);
    UnloadImage(targetImage);

    struct Target target;
    target.pos.x = screenWidth / 2.0;
    target.pos.y = screenHeight / 2.0;
    target.speed.x = screenWidth * 0.004;
    target.speed.y = screenHeight * 0.004;
    target.radius = targetTexture.width / 2.0;

    Image bgImage = LoadImage("resources/pbwmp45x2po71.png");

    struct Vector2 bgSize;
    bgSize.x = ((float)bgImage.width) * 0.01;
    bgSize.y = ((float)bgImage.height) * 0.01;

    if (bgSize.x < screenWidth)
    {
        float ratio = ((float)screenWidth) / bgSize.x;
        bgSize.x *= ratio;
        bgSize.y *= ratio;
    }

    if (bgSize.y < screenHeight)
    {
        float ratio = ((float)screenHeight) / bgSize.y;
        bgSize.x *= ratio;
        bgSize.y *= ratio;
    }

    ImageResizeNN(&bgImage, bgSize.x, bgSize.y);
    Texture2D bg = LoadTextureFromImage(bgImage);
    UnloadImage(bgImage);

    HideCursor();

    enum GameState gameState = MENU;

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {

        if (gameState == PLAY)
        {
            timer = timeLimit + timeBase - GetTime();
            if (timer < 0)
                timer = 0;

            if (timer == 0)
            {
                gameState = MENU;
                continue;
            }

            updateTargetPosition(&target, screenWidth, screenHeight);
        }

        Vector2 mousePos = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (gameState == PLAY)
            {
                float eucDist = calcEucDist(&target.pos, &mousePos);
                if (eucDist <= target.radius)
                {
                    PlaySound(popSound);
                    score++;
                    target.pos.x = (float)generateRandomNumber(target.radius, screenWidth - target.radius);
                    target.pos.y = (float)generateRandomNumber(target.radius, screenHeight - target.radius);
                    if (generateRandomNumber(-5, 5) > 0)
                    {
                        target.speed.x *= -1;
                    }
                    if (generateRandomNumber(-5, 5) > 0)
                    {
                        target.speed.y *= -1;
                    }
                }
            }
            else
            {
                gameState = PLAY;
                timeBase = GetTime();
                score = 0;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(bg, 0, 0, WHITE);

        if (gameState == PLAY)
        {
            DrawCircle(target.pos.x, target.pos.y, target.radius, color1);
            DrawTexture(targetTexture, target.pos.x - target.radius, target.pos.y - target.radius, WHITE);
        }

        snprintf(scoreMessage, sizeof(scoreMessage), "Your score is: %d", score);
        snprintf(timerMessage, sizeof(timerMessage), "You have %d seconds left", timer);
        DrawText(
            scoreMessage,
            textVertOffset,
            textVertOffset,
            24,
            color1);
        DrawText(
            timerMessage,
            screenWidth - 320,
            textVertOffset,
            24,
            color1);
        DrawTexture(crosshairs, mousePos.x - 24, mousePos.y - 24, WHITE);
        if (gameState == MENU)
        {
            DrawText("Click anywhere to begin playing", 190, screenHeight / 2, 30, LIGHTGRAY);
        }
        EndDrawing();
    }

    UnloadTexture(bg);
    UnloadTexture(targetTexture);
    UnloadTexture(crosshairs);
    UnloadSound(popSound);
    CloseAudioDevice();
    CloseWindow(); // Close window and OpenGL context
    return 0;
}
