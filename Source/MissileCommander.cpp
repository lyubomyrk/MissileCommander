// MissileCommander.cpp : Defines the entry point for the application.
//
#include <string>
#include <vector>

#include "MissileCommander.hpp"
#include "raylib.h"
#include "raymath.h"

using namespace std;

struct Missile {
  Vector2 pos;
  Vector2 vel;
  Vector2 dest;
  bool disassembly;
  float radius_dir;
  float explosion_radius;
  float max_radius;
  float growth_speed;
};

int main() {
  const int ScreenWidth = 800;
  const int ScreenHeight = 450;

  InitWindow(ScreenWidth, ScreenHeight, "Missile Commander!");
  SetTargetFPS(60);

  vector<Missile *> missiles;

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    Rectangle ground = {0., ScreenHeight - 50, ScreenWidth, 50};
    Rectangle cannon = {ScreenWidth / 2 - 25, ground.y - 25, 50, 25};
    Vector2 cannon_offset = {25., 25. / 2};
    Vector2 mouse = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 missile_dest = mouse;
      float missile_speed = 300;
      Vector2 cannon_dest_diff = {missile_dest.x - cannon.x - cannon_offset.x,
                                  missile_dest.y - cannon.y};
      Vector2 missile_vel =
          Vector2Scale(Vector2Normalize(cannon_dest_diff), missile_speed);
      Vector2 missile_pos = {cannon.x + cannon_offset.x, cannon.y};
      Missile *new_missile =
          new Missile{missile_pos, missile_vel, missile_dest, false};
      new_missile->max_radius = 25;
      missiles.push_back(new_missile);
    }

    for (auto &missile : missiles) {
      if (!missile->disassembly) {
        missile->pos = Vector2Add(missile->pos, Vector2Scale(missile->vel, dt));
        // BUG if missiles dont explode check here
        if (Vector2Distance(missile->pos, missile->dest) <= 5.) {
          missile->disassembly = true;
          missile->explosion_radius = 0.1;
          missile->radius_dir = 1;
          missile->growth_speed = 10;
        }
      }
      if (missile->disassembly) {
        missile->explosion_radius =
            missile->explosion_radius +
            missile->radius_dir * missile->growth_speed * dt;
        if (missile->explosion_radius >= missile->max_radius) {
          missile->radius_dir = -1.;
        }
      }
    }

    ClearBackground(BLACK);
    DrawFPS(10, 10);
    // draw environment
    DrawRectangleRec(ground, GREEN);
    // draw cannons
    DrawRectangleRec(cannon, BLUE);
    DrawLine(ScreenWidth / 2, 0, ScreenWidth / 2, ScreenHeight, PURPLE);

    // draw crosshair
    DrawLine(mouse.x - 5, mouse.y - 5, mouse.x + 5, mouse.y + 5, RED);
    DrawLine(mouse.x + 5, mouse.y - 5, mouse.x - 5, mouse.y + 5, RED);

    // draw missiles
    for (auto &missile : missiles) {
      if (missile->disassembly) {
        DrawCircle(missile->pos.x, missile->pos.y, missile->explosion_radius,
                   WHITE);
        missile->vel = {
            0.,
            0,
        };
        continue;
      }
      Vector2 line_start = {cannon.x + cannon_offset.x, cannon.y};
      Vector2 line_end = {missile->pos.x, missile->pos.y};
      DrawLineV(line_start, line_end, GREEN);
    }
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
