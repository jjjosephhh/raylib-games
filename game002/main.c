#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "mystructs.h"

enum GameState
{
    MENU,
    PLAY
};

int getRandInt(int min, int max)
{
    return min + rand() % (max - min + 1);
}

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
}

void deactivateIfOutOfBounds(Bullet *bullet, int screenWidth, int screenHeight)
{
    if (bullet->pos.x < 0 || bullet->pos.y < 0 || bullet->pos.x > screenWidth || bullet->pos.y > screenHeight)
    {
        bullet->active = false;
    }
}

float calcDist(Vector2 *p1, Vector2 *p2)
{
    float deltaX = p2->x - p1->x;
    float deltaY = p2->y - p1->y;
    return sqrt(deltaX * deltaX + deltaY * deltaY);
}

int removeInactiveBullets(ZombieManager *zombieManager, BulletManager *bulletManager)
{

    int activeBulletCount = 0;
    for (int i = 0; i < bulletManager->count; i++)
    {
        Bullet *bullet = bulletManager->bullets[i];
        if (bullet == NULL)
            continue;
        deactivateIfOutOfBounds(
            bullet,
            bulletManager->screenWidth,
            bulletManager->screenHeight);

        for (int j = 0; j < zombieManager->count; j++)
        {
            Zombie *zombie = zombieManager->zombies[j];
            if (zombie == NULL || !zombie->active)
                continue;
            float distBtw = calcDist(
                &zombie->posCircle,
                &bullet->pos);
            if (distBtw < (bulletManager->width / 2 + zombieManager->width / 2))
            {
                zombie->active = false;
                bullet->active = false;
            }
        }

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
        }
    }
    bulletManager->count = activeBulletCount;

    int activeZombieCount = 0;
    for (int i = 0; i < zombieManager->count; i++)
    {
        Zombie *zombie = zombieManager->zombies[i];
        if (zombie->active)
        {
            if (activeZombieCount != i)
            {
                Zombie *zombieToFree = zombieManager->zombies[activeZombieCount];
                zombieManager->zombies[activeZombieCount] = zombie;
                zombieManager->zombies[i] = zombieToFree;
            }
            activeZombieCount++;
        }
    }

    for (int i = activeZombieCount; i < zombieManager->count; i++)
    {
        Zombie *zombie = zombieManager->zombies[i];
        if (zombie != NULL && !zombie->active)
        {
            free(zombie);
        }
    }

    int zombiesKilled = zombieManager->count - activeZombieCount;
    zombieManager->count = activeZombieCount;
    return zombiesKilled;
}

void addZombie(ZombieManager *zombieManager, int screenWidth, int screenHeight)
{
    printf("inside the addZombie function\n");

    if (zombieManager->count >= MAX_ZOMBIES)
    {
        return;
    }
    Zombie *zombie = malloc(sizeof(Zombie));

    int side = getRandInt(1, 4);
    if (side == 1)
    {
        zombie->posCircle.x = getRandInt(
            zombieManager->width / 2,
            zombieManager->screenWidth - zombieManager->width / 2);
        zombie->posCircle.y = -zombieManager->height / 2;
    }
    else if (side == 2)
    {
        zombie->posCircle.x = -zombieManager->width / 2;
        zombie->posCircle.y = getRandInt(
            zombieManager->height / 2,
            zombieManager->screenHeight - zombieManager->height / 2);
    }
    else if (side == 3)
    {
        zombie->posCircle.x = getRandInt(
            zombieManager->width / 2,
            zombieManager->screenWidth - zombieManager->width / 2);
        zombie->posCircle.y = zombieManager->screenHeight + zombieManager->height / 2;
    }
    else if (side == 4)
    {
        zombie->posCircle.x = zombieManager->screenWidth + zombieManager->width / 2;
        zombie->posCircle.y = getRandInt(
            zombieManager->height / 2,
            zombieManager->screenHeight - zombieManager->height / 2);
    }
    zombie->posTexture.x = zombie->posCircle.x - zombieManager->width / 2;
    zombie->posTexture.y = zombie->posCircle.y - zombieManager->height / 2;
    zombie->radius = zombieManager->width / 2 + 7;
    zombie->active = true;
    zombieManager->zombies[zombieManager->count] = zombie;
    zombieManager->count++;
}

int main(void)
{
    const int screenWidth = 1000;
    const int screenHeight = 1000;
    InitWindow(screenWidth, screenHeight, "Zombie Shooter Game");
    SetTargetFPS(60);
    InitAudioDevice();

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

    Sound hitSound = LoadSound("resources/mixkit-boxer-getting-hit-2055.wav");
    Sound bloodSound = LoadSound("resources/mixkit-game-blood-pop-slide-2363.wav");
    Sound gunSound = LoadSound("resources/mixkit-small-hit-in-a-game-2072.wav");

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
    bulletManager.width = bulletTexture.width;
    bulletManager.height = bulletTexture.height;
    bulletManager.screenWidth = screenWidth;
    bulletManager.screenHeight = screenHeight;

    ZombieManager zombieManager;
    zombieManager.count = 0;
    zombieManager.width = zombieTexture.width;
    zombieManager.height = zombieTexture.height;
    zombieManager.screenWidth = screenWidth;
    zombieManager.screenHeight = screenHeight;

    float timer = MAX_TIME;

    HideCursor();
    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition();
        float dt = GetFrameTime();
        timer -= dt;
        if (timer <= 0)
        {
            timer = MAX_TIME;
            addZombie(&zombieManager, screenWidth, screenHeight);
        }

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

        int zombiesKilled = removeInactiveBullets(
            &zombieManager,
            &bulletManager);

        if (zombiesKilled > 0)
        {
            PlaySound(hitSound);
            PlaySound(bloodSound);
        }

        for (int i = 0; i < bulletManager.count; i++)
        {
            if (!bulletManager.bullets[i]->active)
                continue;
            bulletManager.bullets[i]->pos.x += bulletManager.bullets[i]->velocity.x * dt;
            bulletManager.bullets[i]->pos.y += bulletManager.bullets[i]->velocity.y * dt;
        }

        for (int i = 0; i < zombieManager.count; i++)
        {
            if (!zombieManager.zombies[i]->active)
                continue;

            float playerZombieAngleRadians = playerMouseAngle(&player.posCircle, &zombieManager.zombies[i]->posCircle);
            double cosPlayerZombie = cos(playerZombieAngleRadians);
            double sinPlayerZombie = sin(playerZombieAngleRadians);
            float velocityX = -cosPlayerZombie * SPEED_ZOMBIE;
            float velocityY = -sinPlayerZombie * SPEED_ZOMBIE;
            double playerZombieAngleDegrees = playerZombieAngleRadians * 180.0 / M_PI;
            zombieManager.zombies[i]->posCircle.x += velocityX * dt;
            zombieManager.zombies[i]->posCircle.y += velocityY * dt;
            zombieManager.zombies[i]->angle = playerZombieAngleDegrees;
        }

        double mouseAngleRadians = playerMouseAngle(&player.posCircle, &mousePos);
        double mouseAngleDegrees = mouseAngleRadians * 180.0 / M_PI;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (bulletManager.count < MAX_BULLETS)
            {
                PlaySound(gunSound);
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

        for (int i = 0; i < zombieManager.count; i++)
        {
            if (!zombieManager.zombies[i]->active)
                continue;
            DrawCircle(
                zombieManager.zombies[i]->posCircle.x,
                zombieManager.zombies[i]->posCircle.y,
                zombieManager.zombies[i]->radius,
                BLACK);
            DrawCircleGradient(
                zombieManager.zombies[i]->posCircle.x,
                zombieManager.zombies[i]->posCircle.y,
                zombieTexture.width / 2 + 3,
                RED,
                ORANGE);
            DrawTexturePro(
                zombieTexture,
                (Rectangle){0, 0, zombieTexture.width, zombieTexture.height},
                (Rectangle){
                    zombieManager.zombies[i]->posCircle.x,
                    zombieManager.zombies[i]->posCircle.y,
                    zombieTexture.width,
                    zombieTexture.height},
                (Vector2){zombieTexture.width / 2, zombieTexture.height / 2},
                zombieManager.zombies[i]->angle,
                WHITE);
        }

        DrawTexture(targetTexture, mousePos.x - targetSize.x, mousePos.y - targetSize.y, WHITE);

        EndDrawing();
    }

    // unload any remaining resources
    UnloadTexture(playerTexture);
    UnloadTexture(zombieTexture);
    UnloadTexture(targetTexture);
    UnloadTexture(bulletTexture);

    UnloadSound(hitSound);
    UnloadSound(bloodSound);
    UnloadSound(gunSound);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}