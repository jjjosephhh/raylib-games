#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_BULLETS 100

enum GameState
{
    MENU,
    PLAY
};
typedef struct
{
    Vector2 posTexture;
    Vector2 posCircle;
    float radius;
    float speed;
} Player;

typedef struct
{
    Vector2 posTexture;
    Vector2 posCircle;
    float radius;
} Zombie;

typedef struct
{
    Zombie *zombies[MAX_BULLETS];
    int count;
} ZombieManager;

typedef struct
{
    Vector2 pos;
    Vector2 velocity;
    float speed;
    double angle;
    bool active;
} Bullet;

typedef struct
{
    Bullet *bullets[MAX_BULLETS];
    int count;
} BulletManager;

float playerMouseAngle(Vector2 *p1, Vector2 *p2)
{
    double deltaX = p2->x - p1->x;
    double deltaY = p2->y - p1->y;
    return atan2(deltaY, deltaX);
}

void addBullet(BulletManager *bulletManager, Bullet *bullet)
{
    bulletManager->bullets[bulletManager->count] = bullet;
    bulletManager->count++;
    printf("bullet count is %d\n", bulletManager->count);
}

void setActive(Bullet *bullet, int screenWidth, int screenHeight)
{
    if (bullet->pos.x < 0 || bullet->pos.y < 0 || bullet->pos.x > screenWidth || bullet->pos.y > screenHeight)
    {
        bullet->active = false;
        printf("bullet %p was set to inactive\n", (void *)bullet);
    }
}

void removeInactiveBullets(BulletManager *bulletManager, int screenWidth, int screenHeight)
{
    int activeBulletCount = 0;
    for (int i = 0; i < bulletManager->count; i++)
    {
        Bullet *bullet = bulletManager->bullets[i];
        setActive(bullet, screenWidth, screenHeight);
        if (bullet->active)
        {
            if (activeBulletCount != i)
            {
                Bullet *bulletToFree = bulletManager->bullets[activeBulletCount];
                bulletManager->bullets[activeBulletCount] = bullet;
                bulletManager->bullets[i] = bulletToFree;
            }
            activeBulletCount++;
        }
    }
    for (int i = activeBulletCount; i < bulletManager->count; i++)
    {
        Bullet *bullet = bulletManager->bullets[i];
        if (bullet != NULL && !bullet->active)
        {
            free(bullet);
            printf("bullet %p was removed\n", (void *)bullet);
        }
    }
    bulletManager->count = activeBulletCount;
}

int main(void)
{
    const int screenWidth = 1000;
    const int screenHeight = 1000;
    InitWindow(screenWidth, screenHeight, "Zombie Shooter Game");
    SetTargetFPS(60);

    // initialize resources
    Image playerImage = LoadImage("resources/icons8-gun-100.png");
    Texture2D playerTexture = LoadTextureFromImage(playerImage);
    UnloadImage(playerImage);

    Image zombieImage = LoadImage("resources/icons8-zombie-100.png");
    Texture2D zombieTexture = LoadTextureFromImage(zombieImage);
    UnloadImage(zombieImage);

    Image targetImage = LoadImage("resources/icons8-target-96.png");
    ImageResize(&targetImage, 25, 25);
    Texture2D targetTexture = LoadTextureFromImage(targetImage);
    UnloadImage(targetImage);

    Image bulletImage = LoadImage("resources/icons8-bullet-100.png");
    ImageResize(&bulletImage, 25, 25);
    Texture2D bulletTexture = LoadTextureFromImage(bulletImage);
    UnloadImage(bulletImage);

    // initialize entities
    Player player;
    player.posCircle.x = screenWidth / 2;
    player.posCircle.y = screenHeight / 2;
    player.posTexture.x = player.posCircle.x - playerTexture.width / 2;
    player.posTexture.y = player.posCircle.y - playerTexture.height / 2;
    player.radius = playerTexture.width / 2 + 7;
    player.speed = 6 * 60;

    Zombie zombie;
    zombie.posCircle.x = screenWidth / 2 + 200;
    zombie.posCircle.y = screenHeight / 2 + 200;
    zombie.posTexture.x = zombie.posCircle.x - zombieTexture.width / 2;
    zombie.posTexture.y = zombie.posCircle.y - zombieTexture.height / 2;
    zombie.radius = zombieTexture.width / 2 + 7;

    Vector2 targetSize;
    targetSize.x = targetTexture.width / 2;
    targetSize.y = targetTexture.height / 2;

    BulletManager bulletManager;
    bulletManager.count = 0;

    BulletManager zombieManager;
    zombieManager.count = 0;

    HideCursor();
    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition();
        float dt = GetFrameTime();

        if (IsKeyDown(KEY_D))
        {
            player.posCircle.x += player.speed * dt;
        }

        if (IsKeyDown(KEY_A))
        {
            player.posCircle.x -= player.speed * dt;
        }

        if (IsKeyDown(KEY_S))
        {
            player.posCircle.y += player.speed * dt;
        }

        if (IsKeyDown(KEY_W))
        {
            player.posCircle.y -= player.speed * dt;
        }

        if (player.posCircle.x - player.radius < 0)
        {
            player.posCircle.x = player.radius + 1;
        }
        if (player.posCircle.y - player.radius < 0)
        {
            player.posCircle.y = player.radius + 1;
        }
        if (player.posCircle.x + player.radius > screenWidth)
        {
            player.posCircle.x = screenWidth - player.radius - 1;
        }
        if (player.posCircle.y + player.radius > screenHeight)
        {
            player.posCircle.y = screenHeight - player.radius - 1;
        }

        for (int i = 0; i < bulletManager.count; i++)
        {
            Bullet *bullet = bulletManager.bullets[i];
            if (bullet->pos.x < 0 || bullet->pos.y < 0 || bullet->pos.x > screenWidth || bullet->pos.y > screenHeight)
            {
                bullet->active = false;
            }
        }

        removeInactiveBullets(&bulletManager, screenWidth, screenHeight);

        for (int i = 0; i < bulletManager.count; i++)
        {
            if (!bulletManager.bullets[i]->active)
                continue;
            bulletManager.bullets[i]->pos.x += bulletManager.bullets[i]->velocity.x * dt;
            bulletManager.bullets[i]->pos.y += bulletManager.bullets[i]->velocity.y * dt;
        }

        double mouseAngleRadians = playerMouseAngle(&player.posCircle, &mousePos);
        double mouseAngleDegrees = mouseAngleRadians * 180.0 / M_PI;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (bulletManager.count < MAX_BULLETS)
            {
                Bullet *bullet = malloc(sizeof(Bullet));
                if (bullet == NULL)
                {
                    printf("Failed to allocate memory for new bullet\n");
                    return 1;
                }
                double cosMouse = cos(mouseAngleRadians);
                double sinMouse = sin(mouseAngleRadians);
                bullet->active = true;
                bullet->pos.x = player.posCircle.x;
                bullet->pos.y = player.posCircle.y;
                bullet->speed = 1000;
                bullet->velocity.x = cosMouse * bullet->speed;
                bullet->velocity.y = sinMouse * bullet->speed;
                bullet->angle = mouseAngleDegrees + 90;
                addBullet(&bulletManager, bullet);
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        DrawCircle(player.posCircle.x, player.posCircle.y, player.radius, BLACK);
        DrawCircleGradient(player.posCircle.x, player.posCircle.y, playerTexture.width / 2 + 3, GREEN, SKYBLUE);

        DrawTexturePro(
            playerTexture,
            (Rectangle){0, 0, playerTexture.width, playerTexture.height},
            (Rectangle){
                player.posCircle.x,
                player.posCircle.y,
                playerTexture.width,
                playerTexture.height},
            (Vector2){playerTexture.width / 2, playerTexture.height / 2},
            mouseAngleDegrees,
            WHITE);

        for (int i = 0; i < bulletManager.count; i++)
        {
            if (!bulletManager.bullets[i]->active)
                continue;
            DrawTexturePro(
                bulletTexture,
                (Rectangle){0, 0, bulletTexture.width, bulletTexture.height},
                (Rectangle){
                    bulletManager.bullets[i]->pos.x,
                    bulletManager.bullets[i]->pos.y,
                    bulletTexture.width,
                    bulletTexture.height},
                (Vector2){bulletTexture.width / 2, bulletTexture.height / 2},
                bulletManager.bullets[i]->angle,
                WHITE);
        }

        DrawCircle(zombie.posCircle.x, zombie.posCircle.y, zombie.radius, BLACK);
        DrawCircleGradient(zombie.posCircle.x, zombie.posCircle.y, zombieTexture.width / 2 + 3, RED, ORANGE);
        DrawTexture(zombieTexture, zombie.posTexture.x, zombie.posTexture.y, WHITE);

        DrawTexture(targetTexture, mousePos.x - targetSize.x, mousePos.y - targetSize.y, WHITE);

        EndDrawing();
    }

    // unload any remaining resources
    UnloadTexture(playerTexture);
    UnloadTexture(zombieTexture);
    UnloadTexture(targetTexture);
    UnloadTexture(bulletTexture);
    CloseWindow();
    return 0;
}