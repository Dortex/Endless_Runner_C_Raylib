#include "raylib.h"
#include <stdio.h>

#define MAX_FRAME_SPEED 15
#define MIN_FRAME_SPEED 1
#define GRAVITY 400
#define JUMP_FORCE -180
#define OBSTACLE_WIDTH 40
#define OBSTACLE_HEIGHT 40
#define OBSTACLE_SPACING_MIN 350
#define OBSTACLE_SPACING_MAX 500

void SaveHighScore(int highScore)
{
    FILE *file = fopen("highscore.txt", "w");
    if (file != NULL)
    {
        fprintf(file, "%d", highScore);
        fclose(file);
    }
}

int LoadHighScore()
{
    int highScore = 0;
    FILE *file = fopen("highscore.txt", "r");
    if (file != NULL)
    {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }
    return highScore;
}

int main()
{
    InitWindow(800, 600, "Run sheep run!");

    Texture2D player = LoadTexture("player_run.png");
    Rectangle frameRec = {0.0f, 0.0f, (float)player.width / 6, (float)player.height};

    int currentFrame = 0;
    int framesCounter = 0;
    int framesSpeed = 8;

    Camera2D camera = {0};
    camera.target = (Vector2){400, 300};
    camera.offset = (Vector2){400, 300};
    camera.rotation = 0.0f;
    camera.zoom = 1.1f;

    int highScore = LoadHighScore();
    bool gameOver = false;

    Vector2 playerPosition;
    float playerVelocity;
    bool isJumping;
    int score;
    float gameSpeed;
    Rectangle obstacles[10];
    int obstacleCount = 10;

    void ResetGame()
    {
        playerPosition = (Vector2){100, 300};
        playerVelocity = 0;
        isJumping = false;
        score = 0;
        gameSpeed = 200;

        for (int i = 0; i < obstacleCount; i++)
        {
            obstacles[i].width = OBSTACLE_WIDTH;
            obstacles[i].height = OBSTACLE_HEIGHT;

            if (i == 0)
            {
                obstacles[i].x = 800 + GetRandomValue(OBSTACLE_SPACING_MIN, OBSTACLE_SPACING_MAX);
            }
            else
            {
                obstacles[i].x = obstacles[i - 1].x + GetRandomValue(OBSTACLE_SPACING_MIN, OBSTACLE_SPACING_MAX);
            }

            obstacles[i].y = 300;
        }
        gameOver = false;
    }

    ResetGame();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (!gameOver)
        {
            // Game logic
            framesCounter++;

            if (framesCounter >= (60 / framesSpeed))
            {
                framesCounter = 0;
                currentFrame++;

                if (currentFrame > 5)
                    currentFrame = 0;

                frameRec.x = (float)currentFrame * (float)player.width / 6;
            }

            if (IsKeyPressed(KEY_SPACE) && !isJumping)
            {
                playerVelocity = JUMP_FORCE;
                isJumping = true;
            }

            playerVelocity += GRAVITY * GetFrameTime();
            playerPosition.y += playerVelocity * GetFrameTime();

            if (playerPosition.y >= 300)
            {
                playerPosition.y = 300;
                playerVelocity = 0;
                isJumping = false;
            }

            for (int i = 0; i < obstacleCount; i++)
            {
                obstacles[i].x -= gameSpeed * GetFrameTime();

                if (obstacles[i].x + obstacles[i].width < 0)
                {
                    float newX = obstacles[(i - 1 + obstacleCount) % obstacleCount].x +
                                 GetRandomValue(OBSTACLE_SPACING_MIN, OBSTACLE_SPACING_MAX);

                    obstacles[i].x = newX;
                    score += 1;
                }

                if (CheckCollisionRecs(
                        (Rectangle){playerPosition.x - frameRec.width / 2, playerPosition.y - frameRec.height / 2, frameRec.width, frameRec.height},
                        obstacles[i]))
                {
                    gameOver = true;
                    if (score > highScore)
                    {
                        highScore = score;
                        SaveHighScore(highScore);
                    }
                }
            }
            gameSpeed += 2 * GetFrameTime();
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (!gameOver)
        {
            BeginMode2D(camera);

            for (int i = 0; i < obstacleCount; i++)
            {
                DrawRectangleRec(obstacles[i], DARKGRAY);
            }

            DrawTextureRec(player, frameRec, (Vector2){playerPosition.x - frameRec.width / 2, playerPosition.y - frameRec.height / 2}, WHITE);
            EndMode2D();
            DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
            DrawText(TextFormat("HighScore: %d", highScore), 10, 40, 20, BLACK);
        }
        else
        {
            DrawText("Game Over!", 300, 200, 40, RED);
            DrawText(TextFormat("Score: %d", score), 300, 250, 30, BLACK);
            DrawText(TextFormat("HighScore: %d", highScore), 300, 300, 30, BLACK);

            DrawText("Press R to Restart", 300, 350, 20, GRAY);

            if (IsKeyPressed(KEY_R))
            {
                ResetGame();
            }
        }

        EndDrawing();
    }

    UnloadTexture(player);
    CloseWindow();

    return 0;
}
