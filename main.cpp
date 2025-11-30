#include "raylib.h"
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

#define MAX_FRUITS 20

int fruits[MAX_FRUITS] = {0};
int fruitTypes[MAX_FRUITS] = {0};
float fruit_posx[MAX_FRUITS];
float fruit_posy[MAX_FRUITS];
Rectangle fruit_hitbox[MAX_FRUITS];

Rectangle player;
float pX = 400;
float pY = 350;

int currentState = MENU;
static int score = 0;

// Game variables
int currentLevel = 1;
float fruitFallSpeed = 2.0f;
int fruitsToCollect = 5;
int fruitsCollected = 0;
int lives = 10;
float playerSpeed = 3.0f;
float speedEffectTimer = 0.0f;
bool isSpeedBoosted = false;
bool gameWon = false;

char SAVE_FILE[20] = "game_save.dat";

bool isFullscreen = false;
const int baseWidth = 800;
const int baseHeight = 450;
int screenWidth = baseWidth;
int screenHeight = baseHeight;

// Render texture for consistent rendering
RenderTexture2D target;
Rectangle sourceRec;
Rectangle destRec;
Vector2 origin;

// Base sizes for game objects
const int basePlayerWidth = 80;
const int basePlayerHeight = 40;
const int baseFruitSize = 25;

// Textures
Texture2D basketTexture;
Texture2D normalFruitTexture;
Texture2D speedUpFruitTexture;
Texture2D speedDownFruitTexture;
Texture2D backgroundTexture;

// Function to load and resize texture
Texture2D LoadAndResizeTexture(const char *fileName, int newWidth, int newHeight)
{
    Image image = LoadImage(fileName);

    if (image.data == NULL)
    {
        UnloadImage(image);
        Image fallback = GenImageColor(newWidth, newHeight, RED);
        Texture2D texture = LoadTextureFromImage(fallback);
        UnloadImage(fallback);
        return texture;
    }

    if (image.width != newWidth || image.height != newHeight)
    {
        ImageResize(&image, newWidth, newHeight);
    }

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    return texture;
}

void InitRenderTexture()
{
    target = LoadRenderTexture(baseWidth, baseHeight);
    sourceRec = {0, 0, (float)baseWidth, (float)-baseHeight};
    destRec = {0, 0, (float)screenWidth, (float)screenHeight};
    origin = {0, 0};
}

void UpdateDestRec()
{
    screenWidth = GetScreenWidth();
    screenHeight = GetScreenHeight();
    destRec = {0, 0, (float)screenWidth, (float)screenHeight};
}

void Create_Fruit(float posx, float posy, int slot, int type = FRUIT_NORMAL)
{
    fruits[slot] = 1;
    fruitTypes[slot] = type;
    fruit_posx[slot] = posx;
    fruit_posy[slot] = posy;
    fruit_hitbox[slot] = (Rectangle){posx, posy, baseFruitSize, baseFruitSize};
}

int GetAvailableFruitSlot()
{
    for (int i = 0; i < MAX_FRUITS; i++)
    {
        if (fruits[i] == 0)
        {
            return i;
        }
    }
    return -1;
}

void SaveGame()
{
    fstream file;
    file.open(SAVE_FILE, ios::out);

    if (file.is_open())
    {
        file << currentLevel;
        file.close();
    }
}

void LoadGame()
{
    fstream file;
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
    for (int i = 0; i < MAX_FRUITS; i++)
    {
        fruits[i] = 0;
        fruitTypes[i] = FRUIT_NORMAL;
        fruit_posx[i] = 0;
        fruit_posy[i] = 0;
    }

    pX = (baseWidth - basePlayerWidth) / 2;
    pY = baseHeight - 100;
    player = (Rectangle){pX, pY, basePlayerWidth, basePlayerHeight};

    score = 0;
    fruitsCollected = 0;
    lives = 10;
    playerSpeed = 3.0f;
    speedEffectTimer = 0.0f;
    isSpeedBoosted = false;
    gameWon = false;

    SetupLevel(currentLevel);
}

void SpawnRandomFruit()
{
    int slot = GetAvailableFruitSlot();
    if (slot != -1)
    {
        float randomX = (float)(rand() % (baseWidth - baseFruitSize));

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

        Create_Fruit(randomX, -20, slot, fruitType);
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

    int centerX = baseWidth / 2;

    DrawText("GAME OVER", centerX - MeasureText("GAME OVER", 40) / 2, baseHeight * 0.2, 40, RED);
    DrawText(TextFormat("Final Score: %d", score), centerX - MeasureText(TextFormat("Final Score: %d", score), 30) / 2, baseHeight * 0.4, 30, WHITE);
    DrawText(TextFormat("Level Reached: %d", currentLevel), centerX - MeasureText(TextFormat("Level Reached: %d", currentLevel), 25) / 2, baseHeight * 0.5, 25, WHITE);

    Rectangle playAgainButton = {centerX - 175, baseHeight * 0.7, 150, 50};
    DrawRectangleRec(playAgainButton, GREEN);
    DrawText("PLAY AGAIN", playAgainButton.x + 15, playAgainButton.y + 15, 20, WHITE);

    Rectangle menuButton = {centerX + 25, baseHeight * 0.7, 150, 50};
    DrawRectangleRec(menuButton, BLUE);
    DrawText("MAIN MENU", menuButton.x + 20, menuButton.y + 15, 20, WHITE);

    Vector2 mousePos = GetMousePosition();
    // Adjust mouse position for render texture
    Vector2 adjustedMousePos = {mousePos.x * baseWidth / screenWidth, mousePos.y * baseHeight / screenHeight};

    if (CheckCollisionPointRec(adjustedMousePos, playAgainButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        ResetGame();
        currentState = GAME;
    }
    if (CheckCollisionPointRec(adjustedMousePos, menuButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        currentState = MENU;
    }
}

void Draw_WinScreen()
{
    ClearBackground(BLACK);

    int centerX = baseWidth / 2;

    DrawText("CONGRATULATIONS!", centerX - MeasureText("CONGRATULATIONS!", 40) / 2, baseHeight * 0.2, 40, GOLD);
    DrawText("YOU'VE WON THE GAME!", centerX - MeasureText("YOU'VE WON THE GAME!", 30) / 2, baseHeight * 0.35, 30, GOLD);
    DrawText(TextFormat("Final Score: %d", score), centerX - MeasureText(TextFormat("Final Score: %d", score), 25) / 2, baseHeight * 0.5, 25, WHITE);

    Rectangle menuButton = {centerX - 100, baseHeight * 0.7, 200, 50};
    DrawRectangleRec(menuButton, BLUE);
    DrawText("MAIN MENU", menuButton.x + 40, menuButton.y + 15, 20, WHITE);

    Vector2 mousePos = GetMousePosition();
    // Adjust mouse position for render texture
    Vector2 adjustedMousePos = {mousePos.x * baseWidth / screenWidth, mousePos.y * baseHeight / screenHeight};

    if (CheckCollisionPointRec(adjustedMousePos, menuButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        currentState = MENU;
    }
}

void Draw_PauseMenu()
{
    // Draw semi-transparent overlay
    DrawRectangle(0, 0, baseWidth, baseHeight, (Color){0, 0, 0, 180});

    int centerX = baseWidth / 2;

    DrawText("GAME PAUSED", centerX - MeasureText("GAME PAUSED", 40) / 2, baseHeight * 0.3, 40, RED);

    Rectangle resumeButton = {centerX - 100, baseHeight * 0.45, 200, 50};
    DrawRectangleRec(resumeButton, GREEN);
    DrawText("RESUME", resumeButton.x + 60, resumeButton.y + 15, 20, WHITE);

    Rectangle menuButton = {centerX - 100, baseHeight * 0.55, 200, 50};
    DrawRectangleRec(menuButton, BLUE);
    DrawText("MAIN MENU", menuButton.x + 40, menuButton.y + 15, 20, WHITE);

    Vector2 mousePos = GetMousePosition();
    // Adjust mouse position for render texture
    Vector2 adjustedMousePos = {mousePos.x * baseWidth / screenWidth, mousePos.y * baseHeight / screenHeight};

    if (CheckCollisionPointRec(adjustedMousePos, resumeButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        currentState = GAME;
    }
    if (CheckCollisionPointRec(adjustedMousePos, menuButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        currentState = MENU;
    }
}

void Draw_Menu()
{
    ClearBackground(BLACK);

    int centerX = baseWidth / 2;

    DrawText("BASKET BLITZ", centerX - MeasureText("BASKET BLITZ", 40) / 2, baseHeight * 0.1, 40, RED);

    Rectangle resetButton = {centerX - 100, baseHeight * 0.3, 200, 50};
    DrawRectangleRec(resetButton, ORANGE);
    DrawText("RESET GAME", resetButton.x + 35, resetButton.y + 15, 20, WHITE);

    Rectangle playButton = {centerX - 100, baseHeight * 0.4, 200, 50};
    DrawRectangleRec(playButton, BLUE);
    DrawText("PLAY GAME", playButton.x + 45, playButton.y + 15, 20, WHITE);

    Rectangle fullscreenButton = {centerX - 100, baseHeight * 0.5, 200, 50};
    DrawRectangleRec(fullscreenButton, isFullscreen ? PURPLE : DARKGRAY);
    DrawText(isFullscreen ? "FULLSCREEN: ON" : "FULLSCREEN: OFF", fullscreenButton.x + 10, fullscreenButton.y + 15, 20, WHITE);

    DrawText(TextFormat("Current Level: %d", currentLevel), 10, 10, 20, YELLOW);

    DrawText("Use A/D keys to move basket", centerX - MeasureText("Use A/D keys to move basket", 20) / 2, screenHeight * 0.75, 20, WHITE);
    DrawText("Green: Normal  Blue: Speed+  Red: Speed-", centerX - MeasureText("Green: Normal  Blue: Speed+  Red: Speed-", 18) / 2, baseHeight * 0.8, 18, WHITE);
    DrawText("Avoid missing fruits or you'll lose lives!", centerX - MeasureText("Avoid missing fruits or you'll lose lives!", 18) / 2, baseHeight * 0.85, 18, WHITE);

    DrawText("Press F to toggle fullscreen", centerX - MeasureText("Press F to toggle fullscreen", 15) / 2, baseHeight * 0.92, 15, GRAY);

    Vector2 mousePos = GetMousePosition();
    // Adjust mouse position for render texture
    Vector2 adjustedMousePos = {mousePos.x * baseWidth / screenWidth, mousePos.y * baseHeight / screenHeight};

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(adjustedMousePos, playButton))
        {
            ResetGame();
            currentState = GAME;
        }
        if (CheckCollisionPointRec(adjustedMousePos, resetButton))
        {
            currentLevel = 1;
            SaveGame();
            ResetGame();
        }
        if (CheckCollisionPointRec(adjustedMousePos, fullscreenButton))
        {
            ToggleFullscreen();
            isFullscreen = !isFullscreen;
            UpdateDestRec();
        }
    }
}

void Draw_Game()
{
    if (IsKeyPressed(KEY_P))
    {
        currentState = PAUSE;
    }

    if (IsKeyPressed(KEY_F))
    {
        ToggleFullscreen();
        isFullscreen = !isFullscreen;
        UpdateDestRec();
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

    if (IsKeyDown(KEY_D))
    {
        pX += playerSpeed;
        if (pX + player.width > baseWidth)
            pX = baseWidth - player.width;
    }
    if (IsKeyDown(KEY_A))
    {
        pX -= playerSpeed;
        if (pX < 0)
            pX = 0;
    }

    player = (Rectangle){pX, pY, basePlayerWidth, basePlayerHeight};

    static int spawnTimer = 0;
    spawnTimer++;

    if (spawnTimer > 90 - (currentLevel * 10) && !gameWon)
    {
        SpawnRandomFruit();
        spawnTimer = 0;
    }

    // Draw background texture
    DrawTexture(backgroundTexture, 0, 0, WHITE);

    DrawText(TextFormat("Score: %d", score), 30, 15, 20, RED);
    DrawText(TextFormat("Level: %d", currentLevel), 30, 45, 20, BLUE);
    DrawText(TextFormat("Fruits: %d/%d", fruitsCollected, fruitsToCollect), 30, 75, 20, DARKGREEN);
    DrawText(TextFormat("Lives: %d", lives), 30, 105, 20, ORANGE);

    if (isSpeedBoosted)
    {
        DrawText("SPEED BOOST!", baseWidth - 150, 55, 15, BLUE);
    }
    else if (playerSpeed < 3.0f)
    {
        DrawText("SLOWED DOWN!", baseWidth - 150, 55, 15, RED);
    }

    DrawText("Press P to Pause", baseWidth - 180, 15, 15, GRAY);
    DrawText("Press F for Fullscreen", baseWidth - 200, 35, 15, GRAY);

    // Draw player texture
    DrawTexture(basketTexture, player.x, player.y, WHITE);

    for (int i = 0; i < MAX_FRUITS; i++)
    {
        if (fruits[i])
        {
            // Draw fruit textures
            Texture2D currentFruitTexture;
            switch (fruitTypes[i])
            {
            case FRUIT_NORMAL:
                currentFruitTexture = normalFruitTexture;
                break;
            case FRUIT_SPEED_UP:
                currentFruitTexture = speedUpFruitTexture;
                break;
            case FRUIT_SPEED_DOWN:
                currentFruitTexture = speedDownFruitTexture;
                break;
            }

            DrawTexture(currentFruitTexture, fruit_posx[i], fruit_posy[i], WHITE);

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
                        gameWon = true;
                        currentState = GAME_OVER;
                    }
                }
            }

            if (fruit_posy[i] > baseHeight)
            {
                if (fruitTypes[i] == FRUIT_NORMAL)
                {
                    lives--;

                    if (lives <= 0)
                    {
                        currentState = GAME_OVER;
                    }
                }

                fruits[i] = 0;
            }
        }
    }
}

int main()
{
    InitWindow(baseWidth, baseHeight, "Basket Blitz");

    // Initialize render texture
    InitRenderTexture();

    // Load textures
    basketTexture = LoadAndResizeTexture("basket.png", basePlayerWidth, basePlayerHeight);
    normalFruitTexture = LoadAndResizeTexture("fruit_normal.png", baseFruitSize, baseFruitSize);
    speedUpFruitTexture = LoadAndResizeTexture("fruit_speedup.png", baseFruitSize, baseFruitSize);
    speedDownFruitTexture = LoadAndResizeTexture("fruit_speeddown.png", baseFruitSize, baseFruitSize);
    backgroundTexture = LoadAndResizeTexture("background.jpg", baseWidth, baseHeight);

    srand(time(NULL));

    LoadGame();

    SetTargetFPS(60);

    pX = (baseWidth - basePlayerWidth) / 2;
    pY = baseHeight - 100;
    player = (Rectangle){pX, pY, basePlayerWidth, basePlayerHeight};

    while (!WindowShouldClose())
    {
        UpdateDestRec();

        // Draw to render texture
        BeginTextureMode(target);

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
            if (gameWon)
            {
                Draw_WinScreen();
            }
            else
            {
                Draw_GameOver();
            }
            break;
        }

        EndTextureMode();

        // Draw render texture to screen
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(target.texture, sourceRec, destRec, origin, 0, WHITE);
        EndDrawing();
    }

    // Unload textures
    UnloadTexture(basketTexture);
    UnloadTexture(normalFruitTexture);
    UnloadTexture(speedUpFruitTexture);
    UnloadTexture(speedDownFruitTexture);
    UnloadTexture(backgroundTexture);

    // Unload render texture
    UnloadRenderTexture(target);

    CloseWindow();

    return 0;
}