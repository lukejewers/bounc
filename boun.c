#include "raylib.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAX_BALLS 15
#define BALL_RADIUS 25
#define FONT_SIZE 20

#define DARK_BG CLITERAL(Color){ 0x18, 0x18, 0x18, 255 }

typedef enum {
    PLAY = 0,
    PAUSE = 1,
} State;

typedef struct Ball {
    int x;
    int y;
    int dx;
    int dy;
} Ball;

typedef struct Sim {
    State state;
    int velocity;
    Ball balls[MAX_BALLS];
    int ball_count;
    int stored_dx;
    int stored_dy;
} Sim;

Sim sim = {
    .state = PLAY,
    .balls = {{
        .x = WINDOW_WIDTH/2,
        .y = WINDOW_HEIGHT/2,
        .dx = 1,
        .dy = 1
    }},
    .ball_count = 1,
    .velocity = 10,
    .stored_dx = 1,
    .stored_dy = 1
};

void UpdateSimState() {
    if (IsKeyPressed(KEY_SPACE)) {
        if (sim.state == PLAY) {
            sim.state = PAUSE;
            sim.stored_dx = sim.balls[0].dx;
            sim.stored_dy = sim.balls[0].dy;
            for (int i = 0; i < sim.ball_count; ++i) {
                sim.balls[i].dx = 0;
                sim.balls[i].dy = 0;
            }
        } else {
            sim.state = PLAY;
            for (int i = 0; i < sim.ball_count; ++i) {
                sim.balls[i].dx = sim.stored_dx;
                sim.balls[i].dy = sim.stored_dy;
            }
        }
    }

    if (sim.state == PAUSE) {
        const char* pauseText = "PAUSED";
        int text_width = MeasureText(pauseText, FONT_SIZE);
        DrawText(pauseText, WINDOW_WIDTH/2 - text_width/2, WINDOW_HEIGHT/2 - FONT_SIZE, FONT_SIZE, WHITE);
    }
}

void UpdateBallPositions() {
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && sim.ball_count < MAX_BALLS) {
        Vector2 vec = GetMousePosition();
        Ball new_ball = {
            .x = vec.x,
            .y = vec.y,
            .dx = 1,
            .dy = 1
        };
        sim.balls[sim.ball_count++] = new_ball;
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

void DrawRedBalls() {
    for (int i = 0; i < sim.ball_count; ++i) {
        DrawCircle(sim.balls[i].x, sim.balls[i].y, BALL_RADIUS, (Color){255, 0, 0, 255});
    }
}

int main(void) {
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
