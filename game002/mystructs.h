#pragma once

#include "constants.h"
#include "raylib.h"

typedef struct
{
    Vector2 posTexture;
    Vector2 posCircle;
    float radius;
    float speed;
    int ammo;
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
    Zombie *zombies;
    int count;
    int countMax;
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
    int damage;
} Bullet;

typedef struct
{
    Bullet *bullets;
    int count;
    int countMax;
    float width;
    float height;
    int screenWidth;
    int screenHeight;
} BulletManager;
