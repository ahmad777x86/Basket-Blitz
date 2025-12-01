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

float scaleX = 1.0f;
float scaleY = 1.0f;

// Base sizes for game objects (in base resolution)
const int basePlayerWidth = 80;
const int basePlayerHeight = 40;
const int baseFruitSize = 25;

// Scaled sizes for game objects
float playerWidth, playerHeight, fruitSize;

// Textures
Texture2D basketTexture;
Texture2D normalFruitTexture;
Texture2D speedUpFruitTexture;
Texture2D speedDownFruitTexture;
Texture2D backgroundTexture;

// Function to load and resize texture
Texture2D LoadAndResizeTexture(char fileName[20], int newWidth, int newHeight)
{
    Image image = LoadImage(fileName);

    // Check if image loaded successfully
    if (image.data == NULL)
    {
        // Create a fallback colored rectangle image
        UnloadImage(image);
        Image fallback = GenImageColor(newWidth, newHeight, RED);
        Texture2D texture = LoadTextureFromImage(fallback);
        UnloadImage(fallback);
        return texture;
    }

    // Resize image if needed
    if (image.width != newWidth || image.height != newHeight)
    {
        ImageResize(&image, newWidth, newHeight);
    }

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    return texture;
}

// Update scaling factors based on current screen size
void UpdateScaling()
{
    scaleX = (float)screenWidth / baseWidth;
    scaleY = (float)screenHeight / baseHeight;

    // Update scaled sizes
    playerWidth = basePlayerWidth * scaleX;
    playerHeight = basePlayerHeight * scaleY;
    fruitSize = baseFruitSize * scaleX;
}

void Create_Fruit(float posx, float posy, int slot, int type = FRUIT_NORMAL)
{
    fruits[slot] = 1;
    fruitTypes[slot] = type;
    fruit_posx[slot] = posx;
    fruit_posy[slot] = posy;
    fruit_hitbox[slot] = (Rectangle){posx, posy, fruitSize, fruitSize};
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
        fruitFallSpeed = 1.5f * scaleY;
        fruitsToCollect = 5;
        break;
    case 2:
        fruitFallSpeed = 2.0f * scaleY;
        fruitsToCollect = 8;
        break;
    case 3:
        fruitFallSpeed = 2.5f * scaleY;
        fruitsToCollect = 12;
        break;
    case 4:
        fruitFallSpeed = 3.0f * scaleY;
        fruitsToCollect = 15;
        break;
    case 5:
        fruitFallSpeed = 3.5f * scaleY;
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

    pX = (screenWidth - playerWidth) / 2;
    pY = screenHeight - 100 * scaleY;
    player = (Rectangle){pX, pY, playerWidth, playerHeight};

    score = 0;
    fruitsCollected = 0;
    lives = 10;
    playerSpeed = 3.0f * scaleX;
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
        float randomX = (float)(rand() % (int)(screenWidth - fruitSize));

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
        playerSpeed = 6.0f * scaleX;
        isSpeedBoosted = true;
        speedEffectTimer = 5.0f;
        break;
    case FRUIT_SPEED_DOWN:
        playerSpeed = 1.5f * scaleX;
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

    int centerX = screenWidth / 2;

    DrawText("GAME OVER", centerX - MeasureText("GAME OVER", 40) / 2, screenHeight * 0.2, 40, RED);
    DrawText(TextFormat("Final Score: %d", score), centerX - MeasureText(TextFormat("Final Score: %d", score), 30) / 2, screenHeight * 0.4, 30, WHITE);
    DrawText(TextFormat("Level Reached: %d", currentLevel), centerX - MeasureText(TextFormat("Level Reached: %d", currentLevel), 25) / 2, screenHeight * 0.5, 25, WHITE);

    Rectangle playAgainButton = {centerX - 175, screenHeight * 0.7, 150, 50};
    DrawRectangleRec(playAgainButton, GREEN);
    DrawText("PLAY AGAIN", playAgainButton.x + 15, playAgainButton.y + 15, 20, WHITE);

    Rectangle menuButton = {centerX + 25, screenHeight * 0.7, 150, 50};
    DrawRectangleRec(menuButton, BLUE);
    DrawText("MAIN MENU", menuButton.x + 20, menuButton.y + 15, 20, WHITE);

    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, playAgainButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        ResetGame();
        currentState = GAME;
    }
    if (CheckCollisionPointRec(mousePos, menuButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        currentState = MENU;
    }
}

void Draw_WinScreen()
{
    ClearBackground(BLACK);

    int centerX = screenWidth / 2;

    DrawText("CONGRATULATIONS!", centerX - MeasureText("CONGRATULATIONS!", 40) / 2, screenHeight * 0.2, 40, GOLD);
    DrawText("YOU'VE WON THE GAME!", centerX - MeasureText("YOU'VE WON THE GAME!", 30) / 2, screenHeight * 0.35, 30, GOLD);
    DrawText(TextFormat("Final Score: %d", score), centerX - MeasureText(TextFormat("Final Score: %d", score), 25) / 2, screenHeight * 0.5, 25, WHITE);

    Rectangle menuButton = {centerX - 100, screenHeight * 0.7, 200, 50};
    DrawRectangleRec(menuButton, BLUE);
    DrawText("MAIN MENU", menuButton.x + 40, menuButton.y + 15, 20, WHITE);

    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, menuButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        currentState = MENU;
    }
}

void Draw_PauseMenu()
{
    ClearBackground((Color){0, 0, 0, 200});

    int centerX = screenWidth / 2;

    DrawText("GAME PAUSED", centerX - MeasureText("GAME PAUSED", 40) / 2, screenHeight * 0.3, 40, RED);

    Rectangle resumeButton = {centerX - 100, screenHeight * 0.45, 200, 50};
    DrawRectangleRec(resumeButton, GREEN);
    DrawText("RESUME", resumeButton.x + 60, resumeButton.y + 15, 20, WHITE);

    Rectangle menuButton = {centerX - 100, screenHeight * 0.55, 200, 50};
    DrawRectangleRec(menuButton, BLUE);
    DrawText("MAIN MENU", menuButton.x + 40, menuButton.y + 15, 20, WHITE);

    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, resumeButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        currentState = GAME;
    }
    if (CheckCollisionPointRec(mousePos, menuButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        currentState = MENU;
    }
}

void Draw_Menu()
{
    ClearBackground(BLACK);

    int centerX = screenWidth / 2;

    DrawText("BASKET BLITZ", centerX - MeasureText("BASKET BLITZ", 40) / 2, screenHeight * 0.1, 40, RED);

    Rectangle resetButton = {centerX - 100, screenHeight * 0.3, 200, 50};
    DrawRectangleRec(resetButton, ORANGE);
    DrawText("RESET GAME", resetButton.x + 35, resetButton.y + 15, 20, WHITE);

    Rectangle playButton = {centerX - 100, screenHeight * 0.4, 200, 50};
    DrawRectangleRec(playButton, BLUE);
    DrawText("PLAY GAME", playButton.x + 45, playButton.y + 15, 20, WHITE);

    Rectangle fullscreenButton = {centerX - 100, screenHeight * 0.5, 200, 50};
    DrawRectangleRec(fullscreenButton, isFullscreen ? PURPLE : DARKGRAY);
    DrawText(isFullscreen ? "FULLSCREEN: ON" : "FULLSCREEN: OFF", fullscreenButton.x + 10, fullscreenButton.y + 15, 20, WHITE);

    DrawText(TextFormat("Current Level: %d", currentLevel), 10, 10, 20, YELLOW);

    DrawText("Use A/D keys to move basket", centerX - MeasureText("Use A/D keys to move basket", 20) / 2, screenHeight * 0.75, 20, WHITE);
    DrawText("Green: Normal  Blue: Speed+  Red: Speed-", centerX - MeasureText("Green: Normal  Blue: Speed+  Red: Speed-", 18) / 2, screenHeight * 0.8, 18, WHITE);
    DrawText("Avoid missing fruits or you'll lose lives!", centerX - MeasureText("Avoid missing fruits or you'll lose lives!", 18) / 2, screenHeight * 0.85, 18, WHITE);

    DrawText("Press F to toggle fullscreen", centerX - MeasureText("Press F to toggle fullscreen", 15) / 2, screenHeight * 0.92, 15, GRAY);

    Vector2 mousePos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
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
        if (CheckCollisionPointRec(mousePos, fullscreenButton))
        {
            ToggleFullscreen();
            isFullscreen = !isFullscreen;
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight();
            UpdateScaling();
        }
    }
}

void Draw_Game()
{
    ClearBackground(BLACK);

    if (IsKeyPressed(KEY_P))
    {
        currentState = PAUSE;
    }

    if (IsKeyPressed(KEY_F))
    {
        ToggleFullscreen();
        isFullscreen = !isFullscreen;
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        UpdateScaling();
    }

    if (speedEffectTimer > 0)
    {
        speedEffectTimer -= GetFrameTime();
        if (speedEffectTimer <= 0)
        {
            playerSpeed = 3.0f * scaleX;
            isSpeedBoosted = false;
        }
    }

    if (IsKeyDown(KEY_D))
    {
        pX += playerSpeed;
        if (pX + player.width > screenWidth)
            pX = screenWidth - player.width;
    }
    if (IsKeyDown(KEY_A))
    {
        pX -= playerSpeed;
        if (pX < 0)
            pX = 0;
    }

    player = (Rectangle){pX, pY, playerWidth, playerHeight};

    static int spawnTimer = 0;
    spawnTimer++;

    if (spawnTimer > (90 - (currentLevel * 10)) / scaleY && !gameWon)
    {
        SpawnRandomFruit();
        spawnTimer = 0;
    }

    DrawText(TextFormat("Score: %d", score), 30 * scaleX, 15 * scaleY, (int)(20 * scaleY), RED);
    DrawText(TextFormat("Level: %d", currentLevel), 30 * scaleX, 45 * scaleY, (int)(20 * scaleY), BLUE);
    DrawText(TextFormat("Fruits: %d/%d", fruitsCollected, fruitsToCollect), 30 * scaleX, 75 * scaleY, (int)(20 * scaleY), DARKGREEN);
    DrawText(TextFormat("Lives: %d", lives), 30 * scaleX, 105 * scaleY, (int)(20 * scaleY), ORANGE);

    if (isSpeedBoosted)
    {
        DrawText("SPEED BOOST!", screenWidth - 150 * scaleX, 55 * scaleY, (int)(15 * scaleY), BLUE);
    }
    else if (playerSpeed < 3.0f * scaleX)
    {
        DrawText("SLOWED DOWN!", screenWidth - 150 * scaleX, 55 * scaleY, (int)(15 * scaleY), RED);
    }

    DrawText("Press P to Pause", screenWidth - 180 * scaleX, 15 * scaleY, (int)(15 * scaleY), GRAY);
    DrawText("Press F for Fullscreen", screenWidth - 200 * scaleX, 35 * scaleY, (int)(15 * scaleY), GRAY);

    // Draw player texture (scaled)
    DrawTexturePro(basketTexture,
                   (Rectangle){0, 0, (float)basketTexture.width, (float)basketTexture.height},
                   (Rectangle){player.x, player.y, playerWidth, playerHeight},
                   (Vector2){0, 0}, 0, WHITE);

    for (int i = 0; i < MAX_FRUITS; i++)
    {
        if (fruits[i])
        {
            // Draw fruit textures (scaled)
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

            DrawTexturePro(currentFruitTexture,
                           (Rectangle){0, 0, (float)currentFruitTexture.width, (float)currentFruitTexture.height},
                           (Rectangle){fruit_posx[i], fruit_posy[i], fruitSize, fruitSize},
                           (Vector2){0, 0}, 0, WHITE);

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

            if (fruit_posy[i] > screenHeight)
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

    UpdateScaling();

    // Load and resize textures to base sizes
    basketTexture = LoadAndResizeTexture("basket.png", basePlayerWidth, basePlayerHeight);
    normalFruitTexture = LoadAndResizeTexture("fruit_normal.png", baseFruitSize, baseFruitSize);
    speedUpFruitTexture = LoadAndResizeTexture("fruit_speedup.png", baseFruitSize, baseFruitSize);
    speedDownFruitTexture = LoadAndResizeTexture("fruit_speeddown.png", baseFruitSize, baseFruitSize);

    srand(time(NULL));

    LoadGame();

    SetTargetFPS(60);

    pX = (baseWidth - basePlayerWidth) / 2;
    pY = baseHeight - 100;
    player = (Rectangle){pX, pY, basePlayerWidth, basePlayerHeight};

    while (!WindowShouldClose())
    {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

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
        EndDrawing();
    }

    // Unload textures
    UnloadTexture(basketTexture);
    UnloadTexture(normalFruitTexture);
    UnloadTexture(speedUpFruitTexture);
    UnloadTexture(speedDownFruitTexture);
    UnloadTexture(backgroundTexture);

    CloseWindow();

    return 0;
}