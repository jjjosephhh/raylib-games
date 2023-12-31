#pragma once

#include "constants.h"
#include "raylib.h"

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
    bool active;
    double angle;
} Zombie;

typedef struct
{
    Zombie *zombies[MAX_ZOMBIES];
    int count;
    float width;
    float height;
    int screenWidth;
    int screenHeight;
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
    float width;
    float height;
    int screenWidth;
    int screenHeight;
} BulletManager;
