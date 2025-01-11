#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PREDKOSC_KLATEK 15
#define MIN_PREDKOSC_KLATEK 1
#define GRAWITACJA 820
#define SILA_SKOKU -280
#define SZEROKOSC_PRZESZKODY 40
#define WYSOKOSC_PRZESZKODY 40
#define DOMYSLNY_MIN_ODSTEP 350
#define DOMYSLNY_MAX_ODSTEP 500
#define MAX_NAJLEPSZE_WYNIKI 3

typedef struct {
    int wynik;
    char data[20];
} NajlepszyWynik;

void ZapiszNajlepszeWyniki(NajlepszyWynik najlepszeWyniki[], int liczbaWynikow)
{
    FILE *file = fopen("resources/najlepszeWyniki.txt", "w");
    if (file != NULL)
    {
        for (int i = 0; i < liczbaWynikow; i++)
        {
            fprintf(file, "%d %s\n", najlepszeWyniki[i].wynik, najlepszeWyniki[i].data);
        }
        fclose(file);
    }
}

void WczytajNajlepszeWyniki(NajlepszyWynik najlepszeWyniki[], int *liczbaWynikow)
{
    FILE *file = fopen("resources/najlepszeWyniki.txt", "r");
    *liczbaWynikow = 0;
    if (file != NULL)
    {
        while (fscanf(file, "%d %19[^\n]", &najlepszeWyniki[*liczbaWynikow].wynik, najlepszeWyniki[*liczbaWynikow].data) == 2 && *liczbaWynikow < MAX_NAJLEPSZE_WYNIKI)
        {
            (*liczbaWynikow)++;
        }
        fclose(file);
    }
}

void DodajNajlepszyWynik(NajlepszyWynik najlepszeWyniki[], int *liczbaWynikow, int nowyWynik)
{
    time_t aktualnyCzas = time(NULL);
    struct tm *t = localtime(&aktualnyCzas);
    char data[20];
    strftime(data, sizeof(data), "%Y-%m-%d %H:%M:%S", t);

    if (*liczbaWynikow < MAX_NAJLEPSZE_WYNIKI || nowyWynik > najlepszeWyniki[MAX_NAJLEPSZE_WYNIKI - 1].wynik)
    {
        if (*liczbaWynikow < MAX_NAJLEPSZE_WYNIKI)
            (*liczbaWynikow)++;

        najlepszeWyniki[*liczbaWynikow - 1].wynik = nowyWynik;
        snprintf(najlepszeWyniki[*liczbaWynikow - 1].data, sizeof(najlepszeWyniki[*liczbaWynikow - 1].data), "%s", data);

        for (int i = *liczbaWynikow - 1; i > 0 && najlepszeWyniki[i].wynik > najlepszeWyniki[i - 1].wynik; i--)
        {
            NajlepszyWynik temp = najlepszeWyniki[i];
            najlepszeWyniki[i] = najlepszeWyniki[i - 1];
            najlepszeWyniki[i - 1] = temp;
        }

        ZapiszNajlepszeWyniki(najlepszeWyniki, *liczbaWynikow);
    }
}

int main()
{
    InitWindow(800, 600, "Run sheep run!");
    InitAudioDevice();

    Sound dzwiekSkoku = LoadSound("resources/jump.wav");
    Texture2D teksturaGracza = LoadTexture("resources/player_run.png");
    Rectangle szerokoscKlatkiAnimacjiGracza = {0.0f, 0.0f, (float)teksturaGracza.width / 6, (float)teksturaGracza.height};
    Texture2D animacjaPorazki = LoadTexture("resources/player_death.png");
    Rectangle szerokoscKlatkiAnimacjiPorazki = {0.0f, 0.0f, (float)animacjaPorazki.width / 6, (float)animacjaPorazki.height};
    Texture2D teksturaPrzeszkody = LoadTexture("resources/fence1.png");
    Texture2D tlo = LoadTexture("resources/summer2.png");
    float tloPozycja = 0.0f;

    int obecnaKlatka = 0;
    int licznikKlatek = 0;
    int predkoscKlatek = 8;

    Camera2D camera = {0};
    camera.target = (Vector2){400, 300};
    camera.offset = (Vector2){500, 300};
    camera.rotation = 0.0f;
    camera.zoom = 1.5f;

    int najlepszeWynikiLiczba = 0;
    NajlepszyWynik najlepszeWyniki[MAX_NAJLEPSZE_WYNIKI];
    WczytajNajlepszeWyniki(najlepszeWyniki, &najlepszeWynikiLiczba);

    bool koniecGry = false;
    bool czyAnimacjaPorazki = false;
    bool czyNajlepszyWynik = false;

    int licznikAnimacjiPorazki = 0;
    int obecnaKlatkaPorazki = 0;

    Vector2 pozycjaGracza;
    float predkoscGracza;
    bool czySkacze;
    int wynik;
    float predkoscGry;
    Rectangle przeszkody[10];
    int liczbaPrzeszkod = 10;

    int dynamicznyMinOdstep = DOMYSLNY_MIN_ODSTEP;
    int dynamicznyMaxOdstep = DOMYSLNY_MAX_ODSTEP;



    void RestartGry()
    {
        pozycjaGracza = (Vector2){100, 400};
        predkoscGracza = 0;
        czySkacze = false;
        wynik = 0;
        predkoscGry = 200;
        czyAnimacjaPorazki = false;
        obecnaKlatkaPorazki = 0;
        czyNajlepszyWynik = false;
        tloPozycja = 0.0f;
        dynamicznyMinOdstep = DOMYSLNY_MIN_ODSTEP;
        dynamicznyMaxOdstep = DOMYSLNY_MAX_ODSTEP;

        for (int i = 0; i < liczbaPrzeszkod; i++)
        {
            przeszkody[i].width = SZEROKOSC_PRZESZKODY;
            przeszkody[i].height = WYSOKOSC_PRZESZKODY;

            if (i == 0)
            {
                przeszkody[i].x = 800 + GetRandomValue(dynamicznyMinOdstep, dynamicznyMaxOdstep);
            }
            else
            {
                przeszkody[i].x = przeszkody[i - 1].x + GetRandomValue(dynamicznyMinOdstep, dynamicznyMaxOdstep);
            }

            przeszkody[i].y = 400;
        }
        koniecGry = false;
    }

    void StartAnimacjaPorazki()
    {
        licznikAnimacjiPorazki++;
        if (licznikAnimacjiPorazki >= (60 / predkoscKlatek))
        {
            licznikAnimacjiPorazki = 0;
            obecnaKlatkaPorazki++;

            if (obecnaKlatkaPorazki >= 6)
            {
                czyAnimacjaPorazki = false;
            }

            szerokoscKlatkiAnimacjiPorazki.x = (float)obecnaKlatkaPorazki * (float)animacjaPorazki.width / 6;
        }
    }
    RestartGry();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (!koniecGry && !czyAnimacjaPorazki)
        {
            licznikKlatek++;

            if (licznikKlatek >= (60 / predkoscKlatek))
            {
                licznikKlatek = 0;
                obecnaKlatka++;

                if (obecnaKlatka > 5)
                    obecnaKlatka = 0;

                szerokoscKlatkiAnimacjiGracza.x = (float)obecnaKlatka * (float)teksturaGracza.width / 6;
            }

            if (IsKeyPressed(KEY_SPACE) && !czySkacze)
            {
                predkoscGracza = SILA_SKOKU;
                czySkacze = true;
                PlaySound(dzwiekSkoku);
            }

            predkoscGracza += GRAWITACJA * GetFrameTime();
            pozycjaGracza.y += predkoscGracza * GetFrameTime();

            if (pozycjaGracza.y >= 400)
            {
                pozycjaGracza.y = 400;
                predkoscGracza = 0;
                czySkacze = false;
            }

            for (int i = 0; i < liczbaPrzeszkod; i++)
            {
                przeszkody[i].x -= predkoscGry * GetFrameTime();

                if (przeszkody[i].x + przeszkody[i].width < 0)
                {
                    float newX = przeszkody[(i - 1 + liczbaPrzeszkod) % liczbaPrzeszkod].x +
                                 GetRandomValue(dynamicznyMinOdstep, dynamicznyMaxOdstep);

                    przeszkody[i].x = newX;
                    wynik += 1;

                    dynamicznyMinOdstep = DOMYSLNY_MIN_ODSTEP + (int)(predkoscGry / 10) * 2;
                    dynamicznyMaxOdstep = DOMYSLNY_MAX_ODSTEP + (int)(predkoscGry / 8) * 2;

                    if (dynamicznyMinOdstep < 200)
                        dynamicznyMinOdstep = 200;

                    if (dynamicznyMaxOdstep < 250)
                        dynamicznyMaxOdstep = 250;
                }

                if (CheckCollisionRecs(
                        (Rectangle){pozycjaGracza.x - szerokoscKlatkiAnimacjiGracza.width / 2, pozycjaGracza.y - szerokoscKlatkiAnimacjiGracza.height / 2, szerokoscKlatkiAnimacjiGracza.width, szerokoscKlatkiAnimacjiGracza.height},
                        przeszkody[i]))
                {
                    koniecGry = true;
                    czyAnimacjaPorazki = true;

                    if (najlepszeWynikiLiczba == 0 || wynik > najlepszeWyniki[0].wynik)
                    {
                        czyNajlepszyWynik = true;
                    }

                    DodajNajlepszyWynik(najlepszeWyniki, &najlepszeWynikiLiczba, wynik);
                }
            }
            predkoscGry += 2 * GetFrameTime();
            tloPozycja -= predkoscGry * GetFrameTime();
            if (tloPozycja <= -tlo.width)
            {
                tloPozycja = 0.0f;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(camera);

        DrawTexture(tlo, (int)tloPozycja, 0, WHITE);
        DrawTexture(tlo, (int)tloPozycja + tlo.width, 0, WHITE);

        if (czyAnimacjaPorazki)
        {
            StartAnimacjaPorazki();
            DrawTextureRec(animacjaPorazki, szerokoscKlatkiAnimacjiPorazki, (Vector2){pozycjaGracza.x - szerokoscKlatkiAnimacjiPorazki.width / 2, pozycjaGracza.y - szerokoscKlatkiAnimacjiPorazki.height / 2}, WHITE);
        }
        else if (!koniecGry)
        {
            for (int i = 0; i < liczbaPrzeszkod; i++)
            {
                DrawTextureEx(teksturaPrzeszkody, (Vector2){przeszkody[i].x, przeszkody[i].y - 20}, 0.0f, SZEROKOSC_PRZESZKODY / (float)teksturaPrzeszkody.width, WHITE);
            }

            DrawTextureRec(teksturaGracza, szerokoscKlatkiAnimacjiGracza, (Vector2){pozycjaGracza.x - szerokoscKlatkiAnimacjiGracza.width / 2, pozycjaGracza.y - szerokoscKlatkiAnimacjiGracza.height / 2}, WHITE);
            EndMode2D();
            DrawText(TextFormat("Wynik: %d", wynik), 10, 10, 20, BLACK);
        }
        else
        {
            EndMode2D();
            DrawText("Przegrana!", 260, 200, 40, RED);
            DrawText(TextFormat("Wynik: %d", wynik), 260, 250, 30, BLACK);

            if (czyNajlepszyWynik)
            {
                DrawText("NOWY REKORD!", 260, 300, 30, GOLD);
            }

            DrawText("Najlepsze wyniki:", 260, 340, 30, DARKGRAY);

            for (int i = 0; i < najlepszeWynikiLiczba; i++)
            {
                DrawText(TextFormat("%d. %d (%s)", i + 1, najlepszeWyniki[i].wynik, najlepszeWyniki[i].data), 260, 380 + i * 30, 20, BLACK);
            }

            DrawText("Restart gry pod przyciskiem 'R'", 260, 500, 20, GOLD);

            if (IsKeyPressed(KEY_R))
            {
                RestartGry();
            }
        }

        EndDrawing();
    }
    UnloadTexture(teksturaGracza);
    UnloadTexture(animacjaPorazki);
    UnloadTexture(teksturaPrzeszkody);
    UnloadTexture(tlo);
    UnloadSound(dzwiekSkoku);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
