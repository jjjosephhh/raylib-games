#pragma once

#include "constants.h"
#include "raylib.h"

typedef enum {
    CATEGORY_BULLET,
    CATEGORY_BULLET_FIRE,
    CATEGORY_BULLET_ICE
} BulletCategory;

typedef struct
{
    Vector2 posTexture;
    Vector2 posCircle;
    float radius;
    float speed;
    int ammo;
    int ammoMax;
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
    BulletCategory bulletCategory;
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
    BulletCategory bulletCategory;
} BulletManager;
