#include "raylib.h"
#include <math.h>
#include <iostream>

using namespace std;

#define MENU 0
#define GAME 1

int fruits[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float fruit_posx[20];
float fruit_posy[20];
Rectangle fruit_hitbox[20];

Rectangle player;
float pX = 400;
float pY = 350;

static int score = 0;

void Create_Fruit(float posx, float posy, int i, float width = 10, float height = 10)
{
    fruits[i] = 1;
    fruit_posx[i] = posx;
    fruit_posy[i] = posy;
    fruit_hitbox[i] = (Rectangle){posx, posy, width, height};
}

void Create_Player(float posx, float posy, float width = 60, float height = 30)
{
    pX = posx;
    pY = posy;
    player = (Rectangle){pX, pY, width, height};
}

void Draw_Menu()
{
}

void Draw_Game()
{
    int i = 0;
    if (IsKeyDown(KEY_D))
    {
        pX += 3;
    }
    if (IsKeyDown(KEY_A))
    {
        pX -= 3;
    }

    player = (Rectangle){pX, pY, player.width, player.height};

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && i < 20)
    {
        Vector2 mousepos = GetMousePosition();
        Create_Fruit(mousepos.x, mousepos.y, i);
        i++;
    }
    ClearBackground(RAYWHITE);
    DrawText(TextFormat("Score: %d", score), 30, 15, 20, RED);
    DrawRectangle(player.x, player.y, player.width, player.height, RED);

    for (int i = 0; i < 20; i++)
    {
        if (fruits[i])
        {
            DrawRectangle(fruit_posx[i], fruit_posy[i], 15, 15, GREEN);
            fruit_posy[i] += 2;
            fruit_hitbox[i].y = fruit_posy[i];
            if (CheckCollisionRecs(fruit_hitbox[i], player))
            {
                fruits[i] = 0;
                fruit_posx[i] = 0;
                fruit_posy[i] = 0;
                score++;
            }
        }
    }
}

int main()
{
    const int width = 800;
    const int height = 450;

    InitWindow(width, height, "Basket Blitz");

    SetTargetFPS(60);
    Create_Player(pX, pY, 60, 30);

    while (!WindowShouldClose())
    {

        BeginDrawing();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}