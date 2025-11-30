#include "raylib.h"
#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace std;

#define MENU 0
#define GAME 1
#define PAUSE 2
#define GAME_OVER 3

// Fruit types
#define FRUIT_NORMAL 0
#define FRUIT_SPEED_UP 1
#define FRUIT_SPEED_DOWN 2

int fruits[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int fruitTypes[20] = {0};
float fruit_posx[20];
float fruit_posy[20];
Rectangle fruit_hitbox[20];
int i = 0;

Rectangle player;
float pX = 400;
float pY = 350;

int currentState = MENU;
static int score = 0;

int currentLevel = 1;
float fruitFallSpeed = 2.0f;
int fruitsToCollect = 5;
int fruitsCollected = 0;
int lives = 10;
float playerSpeed = 3.0f;
float speedEffectTimer = 0.0f;
bool isSpeedBoosted = false;

char SAVE_FILE[15] = "game_save.dat";

void Create_Fruit(float posx, float posy, int i, int type = FRUIT_NORMAL, float width = 15, float height = 15)
{
    fruits[i] = 1;
    fruitTypes[i] = type;
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

void SaveGame()
{
    fstream file(SAVE_FILE);
    file.open(SAVE_FILE, ios::out);

    if (file.is_open())
    {
        file << currentLevel;
        file.close();
    }
}

void LoadGame()
{
    fstream file(SAVE_FILE);
    file.open(SAVE_FILE, ios::in);

    if (file.is_open())
    {
        file >> currentLevel;
        file.close();
    }
    else
    {
        currentLevel = 1;
    }
}

void SetupLevel(int level)
{
    currentLevel = level;
    fruitsCollected = 0;

    switch (level)
    {
    case 1:
        fruitFallSpeed = 1.5f;
        fruitsToCollect = 5;
        break;
    case 2:
        fruitFallSpeed = 2.0f;
        fruitsToCollect = 8;
        break;
    case 3:
        fruitFallSpeed = 2.5f;
        fruitsToCollect = 12;
        break;
    case 4:
        fruitFallSpeed = 3.0f;
        fruitsToCollect = 15;
        break;
    case 5:
        fruitFallSpeed = 3.5f;
        fruitsToCollect = 20;
        break;
    }
}

void ResetGame()
{
    for (int i = 0; i < 20; i++)
    {
        fruits[i] = 0;
        fruitTypes[i] = FRUIT_NORMAL;
        fruit_posx[i] = 0;
        fruit_posy[i] = 0;
    }

    pX = 400;
    pY = 350;
    player = (Rectangle){pX, pY, 60, 30};

    score = 0;
    i = 0;
    fruitsCollected = 0;
    lives = 10;
    playerSpeed = 3.0f;
    speedEffectTimer = 0.0f;
    isSpeedBoosted = false;

    SetupLevel(currentLevel);
}

void SpawnRandomFruit()
{
    if (i < 20)
    {
        float randomX = (float)(rand() % 750);

        int fruitType = FRUIT_NORMAL;
        int randomChance = rand() % 100;

        if (randomChance < 10)
        {
            fruitType = FRUIT_SPEED_UP;
        }
        else if (randomChance < 20)
        {
            fruitType = FRUIT_SPEED_DOWN;
        }

        Create_Fruit(randomX, -20, i, fruitType);
        i++;
    }
}

void ApplySpeedEffect(int type)
{
    switch (type)
    {
    case FRUIT_SPEED_UP:
        playerSpeed = 6.0f;
        isSpeedBoosted = true;
        speedEffectTimer = 5.0f;
        break;
    case FRUIT_SPEED_DOWN:
        playerSpeed = 1.5f;
        isSpeedBoosted = false;
        speedEffectTimer = 3.0f;
        break;
    default:
        break;
    }
}

void Draw_GameOver()
{
    ClearBackground(BLACK);

    DrawText("GAME OVER", 300, 150, 40, RED);
    DrawText(TextFormat("Final Score: %d", score), 320, 220, 30, WHITE);
    DrawText(TextFormat("Level Reached: %d", currentLevel), 300, 260, 25, WHITE);

    Rectangle playAgainButton = {250, 320, 150, 50};
    DrawRectangleRec(playAgainButton, GREEN);
    DrawText("PLAY AGAIN", 260, 335, 20, WHITE);

    Rectangle menuButton = {420, 320, 150, 50};
    DrawRectangleRec(menuButton, BLUE);
    DrawText("MAIN MENU", 440, 335, 20, WHITE);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, playAgainButton))
        {
            ResetGame();
            currentState = GAME;
        }
        if (CheckCollisionPointRec(mousePos, menuButton))
        {
            currentState = MENU;
        }
    }
}

void Draw_PauseMenu()
{
    DrawRectangle(0, 0, 800, 450, (Color){0, 0, 0, 200});

    DrawText("GAME PAUSED", 300, 150, 40, RED);

    Rectangle resumeButton = {300, 220, 200, 50};
    DrawRectangleRec(resumeButton, GREEN);
    DrawText("RESUME", 340, 235, 20, WHITE);

    Rectangle menuButton = {300, 290, 200, 50};
    DrawRectangleRec(menuButton, BLUE);
    DrawText("MAIN MENU", 330, 305, 20, WHITE);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, resumeButton))
        {
            currentState = GAME;
        }
        if (CheckCollisionPointRec(mousePos, menuButton))
        {
            currentState = MENU;
        }
    }
}

void Draw_Menu()
{
    ClearBackground(BLACK);

    DrawText("BASKET BLITZ", 250, 100, 40, RED);

    Rectangle resetButton = {300, 170, 200, 50};
    DrawRectangleRec(resetButton, ORANGE);
    DrawText("RESET GAME", 325, 185, 20, WHITE);

    Rectangle playButton = {300, 240, 200, 50};
    DrawRectangleRec(playButton, BLUE);
    DrawText("PLAY GAME", 325, 255, 20, WHITE);

    DrawText(TextFormat("Current Level: %d", currentLevel), 10, 10, 20, YELLOW);

    DrawText("Use A/D keys to move basket", 250, 350, 20, WHITE);
    DrawText("Green: Normal  Blue: Speed+  Red: Speed-", 230, 380, 18, WHITE);
    DrawText("Avoid missing fruits or you'll lose lives!", 250, 410, 18, WHITE);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, playButton))
        {
            ResetGame();
            currentState = GAME;
        }

        if (CheckCollisionPointRec(mousePos, resetButton))
        {
            currentLevel = 1;
            SaveGame();
            ResetGame();
        }
    }
}

void Draw_Game()
{
    // Pause game with P key
    if (IsKeyPressed(KEY_P))
    {
        currentState = PAUSE;
    }

    if (speedEffectTimer > 0)
    {
        speedEffectTimer -= GetFrameTime();
        if (speedEffectTimer <= 0)
        {
            playerSpeed = 3.0f;
            isSpeedBoosted = false;
        }
    }

    // Bounding Player within screen
    if (IsKeyDown(KEY_D))
    {
        pX += playerSpeed;
        if (pX + player.width > 800)
            pX = 800 - player.width;
    }
    if (IsKeyDown(KEY_A))
    {
        pX -= playerSpeed;
        if (pX < 0)
            pX = 0;
    }

    player = (Rectangle){pX, pY, player.width, player.height};

    static int spawnTimer = 0;
    spawnTimer++;

    if (spawnTimer > 90 - (currentLevel * 10) && i < 20)
    {
        SpawnRandomFruit();
        spawnTimer = 0;
    }

    ClearBackground(RAYWHITE);

    DrawText(TextFormat("Score: %d", score), 30, 15, 20, RED);
    DrawText(TextFormat("Level: %d", currentLevel), 30, 45, 20, BLUE);
    DrawText(TextFormat("Fruits: %d/%d", fruitsCollected, fruitsToCollect), 30, 75, 20, DARKGREEN);
    DrawText(TextFormat("Lives: %d", lives), 30, 105, 20, ORANGE);

    if (isSpeedBoosted)
    {
        DrawText("SPEED BOOST!", 650, 45, 15, BLUE);
    }
    else if (playerSpeed < 3.0f)
    {
        DrawText("SLOWED DOWN!", 650, 45, 15, RED);
    }

    DrawText("Press P to Pause", 600, 15, 15, GRAY);

    DrawRectangle(player.x, player.y, player.width, player.height, RED);

    for (int i = 0; i < 20; i++)
    {
        if (fruits[i])
        {
            Color fruitColor = GREEN;
            if (fruitTypes[i] == FRUIT_SPEED_UP)
            {
                fruitColor = BLUE;
            }
            else if (fruitTypes[i] == FRUIT_SPEED_DOWN)
            {
                fruitColor = RED;
            }

            DrawRectangle(fruit_posx[i], fruit_posy[i], 15, 15, fruitColor);
            fruit_posy[i] += fruitFallSpeed;
            fruit_hitbox[i].y = fruit_posy[i];

            if (CheckCollisionRecs(fruit_hitbox[i], player))
            {
                if (fruitTypes[i] == FRUIT_NORMAL)
                {
                    score++;
                    fruitsCollected++;
                }
                else
                {
                    ApplySpeedEffect(fruitTypes[i]);
                }

                fruits[i] = 0;
                fruit_posx[i] = 0;
                fruit_posy[i] = 0;

                // Level completion
                if (fruitTypes[i] == FRUIT_NORMAL && fruitsCollected >= fruitsToCollect)
                {
                    if (currentLevel < 5)
                    {
                        currentLevel++;
                        SetupLevel(currentLevel);
                        SaveGame();
                    }
                    else
                    {
                        // Game completed
                        DrawText("CONGRATULATIONS! YOU BEAT ALL LEVELS!", 150, 200, 30, RED);
                        if (IsKeyPressed(KEY_ENTER))
                        {
                            currentState = MENU;
                        }
                    }
                }
            }

            // Life losing mechanism
            if (fruit_posy[i] > 450)
            {
                fruits[i] = 0;
                fruit_posx[i] = 0;
                fruit_posy[i] = 0;

                if (fruitTypes[i] == FRUIT_NORMAL)
                {
                    lives--;

                    if (lives <= 0)
                    {
                        currentState = GAME_OVER;
                    }
                }
            }
        }
    }
}

int main()
{
    const int width = 800;
    const int height = 450;

    InitWindow(width, height, "Basket Blitz");
    srand(time(NULL));

    LoadGame();

    SetTargetFPS(60);
    Create_Player(pX, pY, 60, 30);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        switch (currentState)
        {
        case MENU:
            Draw_Menu();
            break;
        case GAME:
            Draw_Game();
            break;
        case PAUSE:
            Draw_PauseMenu();
            break;
        case GAME_OVER:
            Draw_GameOver();
            break;
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}