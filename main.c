#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_FRAME_SPEED 15
#define MIN_FRAME_SPEED 1
#define GRAVITY 820
#define JUMP_FORCE -280
#define OBSTACLE_WIDTH 40
#define OBSTACLE_HEIGHT 40
#define DEFAULT_MIN_SPACE 350
#define DEFAULT_MAX_SPACE 500
#define MAX_AMOUNT_HIGHSCORES 3

typedef struct {
    int score;
    char date[20];
} HighScore;

void SaveHighScores(HighScore highScores[], int scoresAmount)
{
    FILE *file = fopen("resources/highScores.txt", "w");
    if (file != NULL)
    {
        for (int i = 0; i < scoresAmount; i++)
        {
            fprintf(file, "%d %s\n", highScores[i].score, highScores[i].date);
        }
        fclose(file);
    }
}

void LoadHighScores(HighScore highScores[], int *scoresAmount)
{
    FILE *file = fopen("resources/highScores.txt", "r");
    *scoresAmount = 0;
    if (file != NULL)
    {
        while (fscanf(file, "%d %19[^\n]", &highScores[*scoresAmount].score, highScores[*scoresAmount].date) == 2 && *scoresAmount < MAX_AMOUNT_HIGHSCORES)
        {
            (*scoresAmount)++;
        }
        fclose(file);
    }
}

void AddHighScores(HighScore highScores[], int *scoresAmount, int newScore)
{
    time_t timeNow = time(NULL);
    struct tm *t = localtime(&timeNow);
    char date[20];
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", t);

    if (*scoresAmount < MAX_AMOUNT_HIGHSCORES || newScore > highScores[MAX_AMOUNT_HIGHSCORES - 1].score)
    {
        if (*scoresAmount < MAX_AMOUNT_HIGHSCORES)
            (*scoresAmount)++;

        highScores[*scoresAmount - 1].score = newScore;
        snprintf(highScores[*scoresAmount - 1].date, sizeof(highScores[*scoresAmount - 1].date), "%s", date);

        for (int i = *scoresAmount - 1; i > 0 && highScores[i].score > highScores[i - 1].score; i--)
        {
            HighScore temp = highScores[i];
            highScores[i] = highScores[i - 1];
            highScores[i - 1] = temp;
        }

        SaveHighScores(highScores, *scoresAmount);
    }
}

int main()
{
    InitWindow(800, 600, "Run sheep run!");
    InitAudioDevice();

    Sound soundJump = LoadSound("resources/jump.wav");
    Texture2D playerTexture = LoadTexture("resources/player_run.png");
    Rectangle playerAnimFrameWidth = {0.0f, 0.0f, (float)playerTexture.width / 6, (float)playerTexture.height};
    Texture2D deathAnim = LoadTexture("resources/player_death.png");
    Rectangle deathAnimFrameWidth = {0.0f, 0.0f, (float)deathAnim.width / 6, (float)deathAnim.height};
    Texture2D obstacleTexture = LoadTexture("resources/fence1.png");
    Texture2D bg = LoadTexture("resources/summer2.png");
    float bgOffset = 0.0f;

    int currentFrame = 0;
    int frameCounter = 0;
    int frameSpeed = 8;

    Camera2D camera = {0};
    camera.target = (Vector2){400, 300};
    camera.offset = (Vector2){500, 300};
    camera.rotation = 0.0f;
    camera.zoom = 1.5f;

    int amountOfHighscores = 0;
    HighScore highScores[MAX_AMOUNT_HIGHSCORES];
    LoadHighScores(highScores, &amountOfHighscores);

    bool gameOver = false;
    bool playDeathAnim = false;
    bool isHighScore = false;

    int deathAnimCounter = 0;
    int currentDeathFrame = 0;

    Vector2 playerPosition;
    float playerSpeed;
    bool isJump;
    int score;
    float gameSpeed;
    Rectangle obstacles[10];
    int obstacleAmount = 10;

    int dynamicMinSpace = DEFAULT_MIN_SPACE;
    int dynamicMaxSpace = DEFAULT_MAX_SPACE;



    void Restart()
    {
        playerPosition = (Vector2){100, 400};
        playerSpeed = 0;
        isJump = false;
        score = 0;
        gameSpeed = 200;
        playDeathAnim = false;
        currentDeathFrame = 0;
        isHighScore = false;
        bgOffset = 0.0f;
        dynamicMinSpace = DEFAULT_MIN_SPACE;
        dynamicMaxSpace = DEFAULT_MAX_SPACE;

        for (int i = 0; i < obstacleAmount; i++)
        {
            obstacles[i].width = OBSTACLE_WIDTH;
            obstacles[i].height = OBSTACLE_HEIGHT;

            if (i == 0)
            {
                obstacles[i].x = 800 + GetRandomValue(dynamicMinSpace, dynamicMaxSpace);
            }
            else
            {
                obstacles[i].x = obstacles[i - 1].x + GetRandomValue(dynamicMinSpace, dynamicMaxSpace);
            }

            obstacles[i].y = 400;
        }
        gameOver = false;
    }

    void StartDeathAnim()
    {
        deathAnimCounter++;
        if (deathAnimCounter >= (60 / frameSpeed))
        {
            deathAnimCounter = 0;
            currentDeathFrame++;

            if (currentDeathFrame >= 6)
            {
                playDeathAnim = false;
            }

            deathAnimFrameWidth.x = (float)currentDeathFrame * (float)deathAnim.width / 6;
        }
    }
    Restart();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (!gameOver && !playDeathAnim)
        {
            frameCounter++;

            if (frameCounter >= (60 / frameSpeed))
            {
                frameCounter = 0;
                currentFrame++;

                if (currentFrame > 5)
                    currentFrame = 0;

                playerAnimFrameWidth.x = (float)currentFrame * (float)playerTexture.width / 6;
            }

            if (IsKeyPressed(KEY_SPACE) && !isJump)
            {
                playerSpeed = JUMP_FORCE;
                isJump = true;
                PlaySound(soundJump);
            }

            playerSpeed += GRAVITY * GetFrameTime();
            playerPosition.y += playerSpeed * GetFrameTime();

            if (playerPosition.y >= 400)
            {
                playerPosition.y = 400;
                playerSpeed = 0;
                isJump = false;
            }

            for (int i = 0; i < obstacleAmount; i++)
            {
                obstacles[i].x -= gameSpeed * GetFrameTime();

                if (obstacles[i].x + obstacles[i].width < 0)
                {
                    float newX = obstacles[(i - 1 + obstacleAmount) % obstacleAmount].x +
                                 GetRandomValue(dynamicMinSpace, dynamicMaxSpace);

                    obstacles[i].x = newX;
                    score += 1;

                    dynamicMinSpace = DEFAULT_MIN_SPACE + (int)(gameSpeed / 10) * 2;
                    dynamicMaxSpace = DEFAULT_MAX_SPACE + (int)(gameSpeed / 8) * 2;

                    if (dynamicMinSpace < 200)
                        dynamicMinSpace = 200;

                    if (dynamicMaxSpace < 250)
                        dynamicMaxSpace = 250;
                }

                if (CheckCollisionRecs(
                        (Rectangle){playerPosition.x - playerAnimFrameWidth.width / 2, playerPosition.y - playerAnimFrameWidth.height / 2, playerAnimFrameWidth.width, playerAnimFrameWidth.height},
                        obstacles[i]))
                {
                    gameOver = true;
                    playDeathAnim = true;

                    if (amountOfHighscores == 0 || score > highScores[0].score)
                    {
                        isHighScore = true;
                    }

                    AddHighScores(highScores, &amountOfHighscores, score);
                }
            }
            gameSpeed += 2 * GetFrameTime();
            bgOffset -= gameSpeed * GetFrameTime();
            if (bgOffset <= -bg.width)
            {
                bgOffset = 0.0f;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(camera);

        DrawTexture(bg, (int)bgOffset, 0, WHITE);
        DrawTexture(bg, (int)bgOffset + bg.width, 0, WHITE);

        if (playDeathAnim)
        {
            StartDeathAnim();
            DrawTextureRec(deathAnim, deathAnimFrameWidth, (Vector2){playerPosition.x - deathAnimFrameWidth.width / 2, playerPosition.y - deathAnimFrameWidth.height / 2}, WHITE);
        }
        else if (!gameOver)
        {
            for (int i = 0; i < obstacleAmount; i++)
            {
                DrawTextureEx(obstacleTexture, (Vector2){obstacles[i].x, obstacles[i].y - 20}, 0.0f, OBSTACLE_WIDTH / (float)obstacleTexture.width, WHITE);
            }

            DrawTextureRec(playerTexture, playerAnimFrameWidth, (Vector2){playerPosition.x - playerAnimFrameWidth.width / 2, playerPosition.y - playerAnimFrameWidth.height / 2}, WHITE);
            EndMode2D();
            DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
        }
        else
        {
            EndMode2D();
            DrawText("GAME OVER!", 260, 200, 40, RED);
            DrawText(TextFormat("Score: %d", score), 260, 250, 30, BLACK);

            if (isHighScore)
            {
                DrawText("New High Score!", 260, 300, 30, GOLD);
            }

            DrawText("High Scores:", 260, 340, 30, DARKGRAY);

            for (int i = 0; i < amountOfHighscores; i++)
            {
                DrawText(TextFormat("%d. %d (%s)", i + 1, highScores[i].score, highScores[i].date), 260, 380 + i * 30, 20, BLACK);
            }

            DrawText("Press 'R' to restart", 260, 500, 20, GOLD);

            if (IsKeyPressed(KEY_R))
            {
                Restart();
            }
        }

        EndDrawing();
    }
    UnloadTexture(playerTexture);
    UnloadTexture(deathAnim);
    UnloadTexture(obstacleTexture);
    UnloadTexture(bg);
    UnloadSound(soundJump);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
