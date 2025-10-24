#include <math.h>
#include <stdio.h>
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

typedef enum {
    MODE_COLLISION = 0,
    MODE_ABSORPTION = 1,
} Mode;

typedef struct Ball {
    float x;
    float y;
    float dx;
    float dy;
    float radius;
} Ball;

typedef struct Sim {
    State state;
    int speed;
    unsigned int collisions;
    unsigned int absorptions;
    Ball balls[MAX_BALLS];
    Ball paused_balls[MAX_BALLS];
    unsigned int ball_count;
    Mode mode;
} Sim;

Sim sim = {
    .state = PLAY,
    .balls = {{
        .x = WINDOW_WIDTH/2,
        .y = WINDOW_HEIGHT/2,
        .dx = 1.0f,
        .dy = 1.0f,
        .radius = BALL_RADIUS
    }},
    .ball_count = 1,
    .speed = 8,
    .collisions = 0,
    .mode = MODE_COLLISION,
    .absorptions = 0,
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
    if (IsKeyPressed(KEY_UP)) {
        sim.speed++;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        if (sim.speed > 0) sim.speed--;
    }
    if (IsKeyPressed(KEY_SPACE)) {
        if (sim.state == PLAY) {
            sim.state = PAUSE;
            memcpy(sim.paused_balls, sim.balls, sim.ball_count * sizeof(sim.balls[0]));
            for (unsigned int i = 0; i < sim.ball_count; ++i) {
                sim.balls[i].dx = 0;
                sim.balls[i].dy = 0;
            }
        } else {
            sim.state = PLAY;
            for (unsigned int i = 0; i < sim.ball_count; ++i) {
                sim.balls[i].dx = sim.paused_balls[i].dx;
                sim.balls[i].dy = sim.paused_balls[i].dy;
            }
        }
    }
    if (IsKeyPressed(KEY_M)) {
        sim.mode = (sim.mode == MODE_COLLISION) ? MODE_ABSORPTION : MODE_COLLISION;
    }

    if (sim.state == PAUSE) {
        const char* pause_text = "PAUSED";
        int text_width = MeasureText(pause_text, FONT_SIZE);
        DrawText(pause_text, WINDOW_WIDTH/2 - text_width/2, WINDOW_HEIGHT/2 - FONT_SIZE, FONT_SIZE, WHITE);
    }
}

bool CheckBallCollision(Ball a, Ball b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float distance_squared = dx*dx + dy*dy;
    float min_distance = a.radius + b.radius;
    // distance between centers < 2 × BALL_RADIUS
    return distance_squared < min_distance * min_distance;
}

void ResolveBallCollision(Ball *a, Ball *b)
{
    // swap velocity components along collision normal
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float distance = sqrtf(dx*dx + dy*dy);
    float nx = dx / distance;
    float ny = dy / distance;
    // project velocities onto collision normal
    float a_speed = a->dx * nx + a->dy * ny;
    float b_speed = b->dx * nx + b->dy * ny;
    // swap the normal components
    a->dx += (b_speed - a_speed) * nx;
    a->dy += (b_speed - a_speed) * ny;
    b->dx += (a_speed - b_speed) * nx;
    b->dy += (a_speed - b_speed) * ny;
    // incr collisions
    sim.collisions++;
}

void ResolveBallAbsorption(Ball *a, Ball *b)
{
    if (sim.ball_count > 1) {
        a->radius += b->radius * 0.2f;
        *b = sim.balls[sim.ball_count - 1];
        sim.ball_count--;
        sim.absorptions++;
    }
}

void normalise_ball_speed(Ball *new_ball)
{
    float length = sqrtf(new_ball->dx * new_ball->dx + new_ball->dy * new_ball->dy);
    if (length > 0) {
        new_ball->dx /= length;
        new_ball->dy /= length;
    } else {
        new_ball->dx = 1.0f;
        new_ball->dy = 1.0f;
    }
}

void UpdateBallPositions()
{
    if (sim.state == PAUSE) return;
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && sim.ball_count < MAX_BALLS - 1) {
        Vector2 vec = GetMousePosition();
        Ball new_ball = {
            .x = vec.x,
            .y = vec.y,
            .dx = float_rand(-1, 1),
            .dy = float_rand(-1, 1),
            .radius = BALL_RADIUS
        };
        normalise_ball_speed(&new_ball);
        sim.balls[sim.ball_count++] = new_ball;
    }

    for (unsigned int i = 0; i < sim.ball_count; ++i) {
        for (unsigned int j = i + 1; j < sim.ball_count; ++j) {
            if (CheckBallCollision(sim.balls[i], sim.balls[j])) {
                if (sim.mode == MODE_COLLISION) ResolveBallCollision(&sim.balls[i], &sim.balls[j]);
                else if (sim.mode == MODE_ABSORPTION) ResolveBallAbsorption(&sim.balls[i], &sim.balls[j]);
            }
        }
    }

    for (unsigned int i = 0; i < sim.ball_count; ++i) {
        sim.balls[i].x += sim.speed * sim.balls[i].dx;
        sim.balls[i].y += sim.speed * sim.balls[i].dy;

        if (sim.balls[i].y >= WINDOW_HEIGHT - sim.balls[i].radius) sim.balls[i].dy *= -1;
        if (sim.balls[i].x >= WINDOW_WIDTH - sim.balls[i].radius) sim.balls[i].dx *= -1;
        if (sim.balls[i].y <= 0 + sim.balls[i].radius) sim.balls[i].dy *= -1;
        if (sim.balls[i].x <= 0 + sim.balls[i].radius) sim.balls[i].dx *= -1;
    }
}

void DrawBalls()
{
    Color ball_colors[] = {RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, VIOLET};
    for (unsigned int i = 0; i < sim.ball_count; ++i) {
        Color color = ball_colors[i % 7];
        DrawCircle(sim.balls[i].x, sim.balls[i].y, sim.balls[i].radius, color);
    }
}

void DrawHUD()
{
    const int BUFFER_SIZE = 100;
    char buffer[BUFFER_SIZE];
    const int MARGIN = WINDOW_WIDTH / 30;
    const int LINE_HEIGHT = FONT_SIZE * 1.5;

    snprintf(buffer, BUFFER_SIZE, "FPS: %d", GetFPS());
    DrawText(buffer, MARGIN, MARGIN, FONT_SIZE, WHITE);

    snprintf(buffer, BUFFER_SIZE, "speed: %d", sim.speed);
    DrawText(buffer, MARGIN, MARGIN + LINE_HEIGHT, FONT_SIZE, WHITE);

    snprintf(buffer, BUFFER_SIZE, "ball count: %u", sim.ball_count);
    DrawText(buffer, MARGIN, MARGIN + LINE_HEIGHT * 2, FONT_SIZE, WHITE);

   // Add mode indicator
    const char* mode_text = (sim.mode == MODE_COLLISION) ? "COLLISION" : "ABSORPTION";
    snprintf(buffer, BUFFER_SIZE, "mode: %s", mode_text);
    DrawText(buffer, MARGIN, MARGIN + LINE_HEIGHT * 3, FONT_SIZE, WHITE);

    if (sim.mode == MODE_COLLISION) {
        snprintf(buffer, BUFFER_SIZE, "collisions: %u", sim.collisions);
        DrawText(buffer, MARGIN, MARGIN + LINE_HEIGHT * 4, FONT_SIZE, WHITE);
    } else if (sim.mode == MODE_ABSORPTION) {
        snprintf(buffer, BUFFER_SIZE, "absorptions: %u", sim.absorptions);
        DrawText(buffer, MARGIN, MARGIN + LINE_HEIGHT * 4, FONT_SIZE, WHITE);
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
        DrawBalls();
        DrawHUD();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
