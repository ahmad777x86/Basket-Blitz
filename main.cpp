#include "raylib.h"

using namespace std;

int fruits[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int fruit_posx[20];
int fruit_posy[20];

void Create_Fruit(int posx, int posy, int i)
{
    fruits[i] = 1;
    fruit_posx[i] = posx;
    fruit_posy[i] = posy;
}

int main()
{
    const int width = 800;
    const int height = 450;
    int posX = 400;
    int posY = 350;
    static int score = 0;
    int i = 0;

    InitWindow(width, height, "Basket Blitz");

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (IsKeyDown(KEY_D))
        {
            posX += 3;
        }
        if (IsKeyDown(KEY_A))
        {
            posX -= 3;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Vector2 mousepos = GetMousePosition();
            Create_Fruit(mousepos.x, mousepos.y, i);
            i++;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText(TextFormat("%d", score), 400, 50, 20, RED);
        DrawRectangle(posX, posY, 60, 30, RED);

        for (int i = 0; i < 20; i++)
        {
            if (fruits[i])
            {
                DrawCircle(fruit_posx[i], fruit_posy[i], 8, GREEN);
                fruit_posy[i]++;
                if (fruit_posy[i] - posY && fruit_posx[i] == posY)
                {
                    fruits[i] = 0;
                    fruit_posx[i] = 0;
                    fruit_posy[i] = 0;
                    score++;
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}