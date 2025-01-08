#include "raylib.h"
#include <stdio.h>

#define MAX_FRAME_SPEED 15
#define MIN_FRAME_SPEED 1
#define GRAVITY 820
#define JUMP_FORCE -280
#define OBSTACLE_WIDTH 40
#define OBSTACLE_HEIGHT 40
#define BASE_OBSTACLE_SPACING_MIN 350
#define BASE_OBSTACLE_SPACING_MAX 500

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
    InitAudioDevice();

    Sound jump = LoadSound("jump.wav");

    Texture2D player = LoadTexture("player_run.png");
    Rectangle frameRec = {0.0f, 0.0f, (float)player.width / 6, (float)player.height};
    Texture2D playerDeath = LoadTexture("player_death.png");
    Rectangle deathFrameRec = {0.0f, 0.0f, (float)playerDeath.width / 6, (float)playerDeath.height};
    Texture2D obstacleTexture = LoadTexture("fence1.png");
    Texture2D background = LoadTexture("summer2.png");
    float backgroundOffset = 0.0f;

    int currentFrame = 0;
    int framesCounter = 0;
    int framesSpeed = 8;

    Camera2D camera = {0};
    camera.target = (Vector2){400, 300};
    camera.offset = (Vector2){400, 300};
    camera.rotation = 0.0f;
    camera.zoom = 1.2f;

    int highScore = LoadHighScore();
    bool gameOver = false;
    bool isPlayingDeathAnimation = false;

    int deathFrameCounter = 0;
    int currentDeathFrame = 0;

    Vector2 playerPosition;
    float playerVelocity;
    bool isJumping;
    int score;
    float gameSpeed;
    Rectangle obstacles[10];
    int obstacleCount = 10;

    int dynamicSpacingMin = BASE_OBSTACLE_SPACING_MIN;
    int dynamicSpacingMax = BASE_OBSTACLE_SPACING_MAX;

    void ResetGame()
    {
        playerPosition = (Vector2){100, 400};
        playerVelocity = 0;
        isJumping = false;
        score = 0;
        gameSpeed = 200;
        isPlayingDeathAnimation = false;
        currentDeathFrame = 0;
        backgroundOffset = 0.0f;
        dynamicSpacingMin = BASE_OBSTACLE_SPACING_MIN;
        dynamicSpacingMax = BASE_OBSTACLE_SPACING_MAX;

        for (int i = 0; i < obstacleCount; i++)
        {
            obstacles[i].width = OBSTACLE_WIDTH;
            obstacles[i].height = OBSTACLE_HEIGHT;

            if (i == 0)
            {
                obstacles[i].x = 800 + GetRandomValue(dynamicSpacingMin, dynamicSpacingMax);
            }
            else
            {
                obstacles[i].x = obstacles[i - 1].x + GetRandomValue(dynamicSpacingMin, dynamicSpacingMax);
            }

            obstacles[i].y = 400;
        }
        gameOver = false;
    }

    void PlayDeathAnimation()
    {
        deathFrameCounter++;
        if (deathFrameCounter >= (60 / framesSpeed))
        {
            deathFrameCounter = 0;
            currentDeathFrame++;

            if (currentDeathFrame >= 6)
            {
                isPlayingDeathAnimation = false;
            }

            deathFrameRec.x = (float)currentDeathFrame * (float)playerDeath.width / 6;
        }
    }
    ResetGame();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (!gameOver && !isPlayingDeathAnimation)
        {
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
                PlaySound(jump);
            }

            playerVelocity += GRAVITY * GetFrameTime();
            playerPosition.y += playerVelocity * GetFrameTime();

            if (playerPosition.y >= 400)
            {
                playerPosition.y = 400;
                playerVelocity = 0;
                isJumping = false;
            }

            for (int i = 0; i < obstacleCount; i++)
            {
                obstacles[i].x -= gameSpeed * GetFrameTime();

                if (obstacles[i].x + obstacles[i].width < 0)
                {
                    float newX = obstacles[(i - 1 + obstacleCount) % obstacleCount].x +
                                 GetRandomValue(dynamicSpacingMin, dynamicSpacingMax);

                    obstacles[i].x = newX;
                    score += 1;

                    dynamicSpacingMin = BASE_OBSTACLE_SPACING_MIN + (int)(gameSpeed / 10) *2;
                    dynamicSpacingMax = BASE_OBSTACLE_SPACING_MAX + (int)(gameSpeed / 8) *2;

                    if (dynamicSpacingMin < 200)
                        dynamicSpacingMin = 200;

                    if (dynamicSpacingMax < 250)
                        dynamicSpacingMax = 250;
                }

                if (CheckCollisionRecs(
                        (Rectangle){playerPosition.x - frameRec.width / 2, playerPosition.y - frameRec.height / 2, frameRec.width, frameRec.height},
                        obstacles[i]))
                {
                    gameOver = true;
                    isPlayingDeathAnimation = true;
                    if (score > highScore)
                    {
                        highScore = score;
                        SaveHighScore(highScore);
                    }
                }
            }
            gameSpeed += 2 * GetFrameTime();
            backgroundOffset -= gameSpeed * GetFrameTime();
            if (backgroundOffset <= -background.width)
            {
                backgroundOffset = 0.0f;
            }
        }

        BeginDrawing();
        //ClearBackground(RAYWHITE);
        DrawTexture(background, (int)backgroundOffset, 0, WHITE);
        DrawTexture(background, (int)backgroundOffset + background.width, 0, WHITE);

        if (isPlayingDeathAnimation)
        {
            PlayDeathAnimation();
            DrawTextureRec(playerDeath, deathFrameRec, (Vector2){playerPosition.x - deathFrameRec.width / 2, playerPosition.y - deathFrameRec.height / 2}, WHITE);
        }
        else if (!gameOver)
        {
            BeginMode2D(camera);

            for (int i = 0; i < obstacleCount; i++)
            {
                DrawTextureEx(obstacleTexture, (Vector2){obstacles[i].x, obstacles[i].y - 20}, 0.0f, OBSTACLE_WIDTH / (float)obstacleTexture.width, WHITE);
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
    UnloadTexture(playerDeath);
    UnloadTexture(obstacleTexture);
    UnloadTexture(background);
    UnloadSound(jump);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
