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
        Bullet *bullet = &bulletManager->bullets[i];
        deactivateIfOutOfBounds(
            bullet,
            bulletManager->screenWidth,
            bulletManager->screenHeight);

        for (int j = 0; j < zombieManager->count; j++)
        {
            Zombie *zombie = &zombieManager->zombies[j];
            if (!zombie->active)
                continue;
            float distBtw = calcDist(&zombie->posCircle, &bullet->pos);
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
                Bullet bulletToFree = bulletManager->bullets[activeBulletCount];
                bulletManager->bullets[activeBulletCount] = *bullet;
                bulletManager->bullets[i] = bulletToFree;
            }
            activeBulletCount++;
        }
    }
    bulletManager->count = activeBulletCount;
    int activeZombieCount = 0;
    for (int i = 0; i < zombieManager->count; i++)
    {
        Zombie zombie = zombieManager->zombies[i];
        if (zombie.active)
        {
            if (activeZombieCount != i)
            {
                Zombie zombieToFree = zombieManager->zombies[activeZombieCount];
                zombieManager->zombies[activeZombieCount] = zombie;
                zombieManager->zombies[i] = zombieToFree;
            }
            activeZombieCount++;
        }
    }

    int zombiesKilled = zombieManager->count - activeZombieCount;
    zombieManager->count = activeZombieCount;
    return zombiesKilled;
}

void addZombie(ZombieManager *zombieManager, int screenWidth, int screenHeight)
{
    Zombie zombie;
    int side = getRandInt(1, 4);
    if (side == 1)
    {
        zombie.posCircle.x = getRandInt(
            zombieManager->width / 2,
            zombieManager->screenWidth - zombieManager->width / 2);
        zombie.posCircle.y = -zombieManager->height / 2;
    }
    else if (side == 2)
    {
        zombie.posCircle.x = -zombieManager->width / 2;
        zombie.posCircle.y = getRandInt(
            zombieManager->height / 2,
            zombieManager->screenHeight - zombieManager->height / 2);
    }
    else if (side == 3)
    {
        zombie.posCircle.x = getRandInt(
            zombieManager->width / 2,
            zombieManager->screenWidth - zombieManager->width / 2);
        zombie.posCircle.y = zombieManager->screenHeight + zombieManager->height / 2;
    }
    else if (side == 4)
    {
        zombie.posCircle.x = zombieManager->screenWidth + zombieManager->width / 2;
        zombie.posCircle.y = getRandInt(
            zombieManager->height / 2,
            zombieManager->screenHeight - zombieManager->height / 2);
    }
    zombie.posTexture.x = zombie.posCircle.x - zombieManager->width / 2;
    zombie.posTexture.y = zombie.posCircle.y - zombieManager->height / 2;
    zombie.radius = zombieManager->width / 2 + 7;
    zombie.active = true;
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

    Image ammoImage = LoadImage("resources/icons8-bullet-67.png");
    ImageResize(&ammoImage, 60, 60);
    Texture2D ammoTexture = LoadTextureFromImage(ammoImage);
    UnloadImage(ammoImage);

    Sound hitSound = LoadSound("resources/mixkit-boxer-getting-hit-2055.wav");
    Sound bloodSound = LoadSound("resources/mixkit-game-blood-pop-slide-2363.wav");
    Sound gunSound = LoadSound("resources/mixkit-small-hit-in-a-game-2072.wav");
    Sound gunCockSound = LoadSound("resources/revolvercock1-6924.mp3");
    Sound reloadSound = LoadSound("resources/1911-reload-6248.mp3");

    // initialize entities
    Player player;
    player.ammoMax = 5;
    player.ammo = player.ammoMax;
    player.posCircle.x = screenWidth / 2;
    player.posCircle.y = screenHeight / 2;
    player.posTexture.x = player.posCircle.x - playerTexture.width / 2;
    player.posTexture.y = player.posCircle.y - playerTexture.height / 2;
    player.radius = playerTexture.width / 2 + 7;
    player.speed = 6 * 60;

    Vector2 targetSize;
    targetSize.x = targetTexture.width / 2;
    targetSize.y = targetTexture.height / 2;

    BulletManager bulletManager;
    bulletManager.bullets = NULL;
    bulletManager.count = 0;
    bulletManager.countMax = 0;
    bulletManager.width = bulletTexture.width;
    bulletManager.height = bulletTexture.height;
    bulletManager.screenWidth = screenWidth;
    bulletManager.screenHeight = screenHeight;

    ZombieManager zombieManager;
    zombieManager.zombies = NULL;
    zombieManager.count = 0;
    zombieManager.countMax = 0;
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
            if (zombieManager.count + 1 > zombieManager.countMax)
            {
                zombieManager.countMax = 2 * zombieManager.count + 1;
                zombieManager.zombies = realloc(zombieManager.zombies, zombieManager.countMax * sizeof(Zombie));
            }
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

        if (IsKeyReleased(KEY_SPACE))
        {
            player.ammo = player.ammoMax;
            PlaySound(reloadSound);
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
            Bullet bullet = bulletManager.bullets[i];
            if (bullet.pos.x < 0 || bullet.pos.y < 0 || bullet.pos.x > screenWidth || bullet.pos.y > screenHeight)
            {
                bullet.active = false;
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
            if (!bulletManager.bullets[i].active)
                continue;
            bulletManager.bullets[i].pos.x += bulletManager.bullets[i].velocity.x * dt;
            bulletManager.bullets[i].pos.y += bulletManager.bullets[i].velocity.y * dt;
        }

        for (int i = 0; i < zombieManager.count; i++)
        {
            Zombie *zombie = &zombieManager.zombies[i];
            if (!zombie->active)
                continue;

            float playerZombieAngleRadians = playerMouseAngle(&player.posCircle, &zombie->posCircle);
            double cosPlayerZombie = cos(playerZombieAngleRadians);
            double sinPlayerZombie = sin(playerZombieAngleRadians);
            float velocityX = -cosPlayerZombie * SPEED_ZOMBIE;
            float velocityY = -sinPlayerZombie * SPEED_ZOMBIE;
            double playerZombieAngleDegrees = playerZombieAngleRadians * 180.0 / M_PI;
            zombie->posCircle.x += velocityX * dt;
            zombie->posCircle.y += velocityY * dt;
            zombie->angle = playerZombieAngleDegrees;
        }

        double mouseAngleRadians = playerMouseAngle(&player.posCircle, &mousePos);
        double mouseAngleDegrees = mouseAngleRadians * 180.0 / M_PI;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {

            if (bulletManager.count + 1 > bulletManager.countMax)
            {
                bulletManager.countMax = 2 * bulletManager.count + 1;
                bulletManager.bullets = realloc(
                    bulletManager.bullets,
                    bulletManager.countMax * sizeof(Bullet));
            }

            if (player.ammo > 0)
            {
                PlaySound(gunSound);
                Bullet bullet;
                double cosMouse = cos(mouseAngleRadians);
                double sinMouse = sin(mouseAngleRadians);
                bullet.active = true;
                bullet.pos.x = player.posCircle.x;
                bullet.pos.y = player.posCircle.y;
                bullet.speed = 1000;
                bullet.velocity.x = cosMouse * bullet.speed;
                bullet.velocity.y = sinMouse * bullet.speed;
                bullet.angle = mouseAngleDegrees + 90;
                bulletManager.bullets[bulletManager.count] = bullet;
                bulletManager.count++;
                player.ammo--;
            }
            else
            {
                PlaySound(gunCockSound);
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
            if (!bulletManager.bullets[i].active)
                continue;
            DrawTexturePro(
                bulletTexture,
                (Rectangle){0, 0, bulletTexture.width, bulletTexture.height},
                (Rectangle){
                    bulletManager.bullets[i].pos.x,
                    bulletManager.bullets[i].pos.y,
                    bulletTexture.width,
                    bulletTexture.height},
                (Vector2){bulletTexture.width / 2, bulletTexture.height / 2},
                bulletManager.bullets[i].angle,
                WHITE);
        }

        for (int i = 0; i < zombieManager.count; i++)
        {
            Zombie zombie = zombieManager.zombies[i];
            if (!zombie.active)
                continue;
            DrawCircle(
                zombie.posCircle.x,
                zombie.posCircle.y,
                zombie.radius,
                BLACK);
            DrawCircleGradient(
                zombie.posCircle.x,
                zombie.posCircle.y,
                zombieTexture.width / 2 + 3,
                RED,
                ORANGE);
            DrawTexturePro(
                zombieTexture,
                (Rectangle){0, 0, zombieTexture.width, zombieTexture.height},
                (Rectangle){
                    zombie.posCircle.x,
                    zombie.posCircle.y,
                    zombieTexture.width,
                    zombieTexture.height},
                (Vector2){zombieTexture.width / 2, zombieTexture.height / 2},
                zombie.angle,
                WHITE);
        }

        DrawTexture(targetTexture, mousePos.x - targetSize.x, mousePos.y - targetSize.y, WHITE);
        for (int i = 0; i < player.ammoMax; i++)
        {
            if (i < player.ammo)
            {
                DrawTexture(
                    ammoTexture,
                    screenWidth - (i + 1) * (ammoTexture.width - 10),
                    screenHeight - ammoTexture.height - 10,
                    WHITE);
            }
            else
            {
                DrawTexturePro(ammoTexture,
                               (Rectangle){0, 0, ammoTexture.width, ammoTexture.height},
                               (Rectangle){
                                   screenWidth - (i + 1) * (ammoTexture.width - 10),
                                   screenHeight - ammoTexture.height - 10,
                                   ammoTexture.width,
                                   ammoTexture.height},
                               (Vector2){0, 0},
                               0.0f,
                               Fade(WHITE, 0.5f));
            }
        }
        EndDrawing();
    }

    // unload any remaining resources
    UnloadTexture(playerTexture);
    UnloadTexture(zombieTexture);
    UnloadTexture(targetTexture);
    UnloadTexture(bulletTexture);
    UnloadTexture(ammoTexture);

    UnloadSound(hitSound);
    UnloadSound(bloodSound);
    UnloadSound(gunSound);
    UnloadSound(gunCockSound);
    UnloadSound(reloadSound);

    // free any memory
    free(bulletManager.bullets);
    free(zombieManager.zombies);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}