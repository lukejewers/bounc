#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "raylib.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAX_BALLS 20
#define BALL_RADIUS 15
#define FONT_SIZE 20

#define DARK_BG CLITERAL(Color){ 0x18, 0x18, 0x18, 255 }

typedef enum {
    PLAY = 0,
    PAUSE = 1,
} State;

typedef struct Ball {
    int x;
    int y;
    float dx;
    float dy;
} Ball;

typedef struct Sim {
    State state;
    int velocity;
    Ball balls[MAX_BALLS];
    Ball paused_balls[MAX_BALLS];
    int ball_count;
} Sim;

Sim sim = {
    .state = PLAY,
    .balls = {{
        .x = WINDOW_WIDTH/2,
        .y = WINDOW_HEIGHT/2,
        .dx = 1.0f,
        .dy = 1.0f
    }},
    .ball_count = 1,
    .velocity = 8,
};

float float_rand(float min, float max)
{
    float scale = rand() / (float)RAND_MAX;
    float r = min + scale * (max - min);
    // avoid values too close to 0
    if (fabsf(r) < 0.3f) r = (r >= 0) ? 0.3f : -0.3f;
    return r;
}

void UpdateSimState()
{
    if (IsKeyPressed(KEY_SPACE)) {
        if (sim.state == PLAY) {
            sim.state = PAUSE;
            memcpy(sim.paused_balls, sim.balls, sim.ball_count * sizeof(sim.balls[0]));
            for (int i = 0; i < sim.ball_count; ++i) {
                sim.balls[i].dx = 0;
                sim.balls[i].dy = 0;
            }
        } else {
            sim.state = PLAY;
            for (int i = 0; i < sim.ball_count; ++i) {
                sim.balls[i].dx = sim.paused_balls[i].dx;
                sim.balls[i].dy = sim.paused_balls[i].dy;
            }
        }
    }

    if (sim.state == PAUSE) {
        const char* pauseText = "PAUSED";
        int text_width = MeasureText(pauseText, FONT_SIZE);
        DrawText(pauseText, WINDOW_WIDTH/2 - text_width/2, WINDOW_HEIGHT/2 - FONT_SIZE, FONT_SIZE, WHITE);
    }
}

bool CheckBallCollision(Ball a, Ball b)
{
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    int distance_squared = dx*dx + dy*dy;
    // distance between centers < 2 × BALL_RADIUS
    return distance_squared < (BALL_RADIUS * 2) * (BALL_RADIUS * 2);
}

void ResolveBallCollision(Ball *a, Ball *b)
{
    // swap velocities
    int temp_adx = a->dx;
    int temp_ady = a->dy;
    a->dx = b->dx;
    a->dy = b->dy;
    b->dx = temp_adx;
    b->dy = temp_ady;

    // move balls apart to prevent sticking
    a->x += a->dx * sim.velocity;
    a->y += a->dy * sim.velocity;
    b->x += b->dx * sim.velocity;
    b->y += b->dy * sim.velocity;
}

void normalise_ball_speed(Ball *new_ball)
{
    float length = sqrtf(new_ball->dx * new_ball->dx + new_ball->dy * new_ball->dy);
    if (length > 0) {
        new_ball->dx /= length;
        new_ball->dy /= length;
    }
}

void UpdateBallPositions()
{
    if (sim.state == PAUSE) return;
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && sim.ball_count < MAX_BALLS) {
        Vector2 vec = GetMousePosition();
        Ball new_ball = {
            .x = vec.x,
            .y = vec.y,
            .dx = float_rand(-1, 1),
            .dy = float_rand(-1, 1)
        };
        normalise_ball_speed(&new_ball);
        sim.balls[sim.ball_count++] = new_ball;
    }

    for (int i = 0; i < sim.ball_count; ++i) {
        for (int j = i + 1; j < sim.ball_count; ++j) {
            if (CheckBallCollision(sim.balls[i], sim.balls[j])) {
                ResolveBallCollision(&sim.balls[i], &sim.balls[j]);
            }
        }
    }

    for (int i = 0; i < sim.ball_count; ++i) {
        sim.balls[i].x += sim.velocity * sim.balls[i].dx;
        sim.balls[i].y += sim.velocity * sim.balls[i].dy;

        if (sim.balls[i].y >= WINDOW_HEIGHT - BALL_RADIUS) sim.balls[i].dy = -1;
        if (sim.balls[i].x >= WINDOW_WIDTH - BALL_RADIUS) sim.balls[i].dx = -1;
        if (sim.balls[i].y <= 0 + BALL_RADIUS) sim.balls[i].dy = 1;
        if (sim.balls[i].x <= 0 + BALL_RADIUS) sim.balls[i].dx = 1;
    }
}

void DrawRedBalls()
{
    for (int i = 0; i < sim.ball_count; ++i) {
        DrawCircle(sim.balls[i].x, sim.balls[i].y, BALL_RADIUS, (Color){255, 0, 0, 255});
    }
}

int main(void)
{
    srand(time(NULL));

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "bounc");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        // Update
        UpdateSimState();
        UpdateBallPositions();
        // Draw
        ClearBackground(DARK_BG);
        DrawRedBalls();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
