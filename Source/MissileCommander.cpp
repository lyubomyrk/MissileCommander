// MissileCommander.cpp : Defines the entry point for the application.
//
#include <string>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#include "MissileCommander.hpp"
#include "WELL512.hpp"

using namespace std;

struct Silo {
  Vector2 pos;
  int missiles;
};

// Generic representation of every missile in game
struct Missile {
  Vector2 ipos;
  Vector2 pos;
  Vector2 vel;
  Vector2 dest;
};

// Generic representation of every explosion in game
struct Explosion {
  Vector2 pos;
  float rad;
  float max_rad;
  float growth_dir; // 1 outward, -1 inward
  float growth_speed;
};

const int ScreenWidth = 800;
const int ScreenHeight = 450;

const Vector2 ground_dims = {ScreenWidth, 50.};

const int starting_missile_amount = -1.;
const Vector2 silo_dims = {50., 25.};
const Vector2 silo_offset = {silo_dims.x / 2, silo_dims.y / 2};
const float cannon_length = 30;
const float cannon_thickness = 5;

const float missile_speed = 250;
const float missile_dest_tolerance = 5.;

const float explosion_growth_speed = 50.;
const float explosion_max_rad = 40.;

const float enemy_missile_speed = 50.;
const float enemy_missile_spawn_time = 2.5;

int main() {

  InitWindow(ScreenWidth, ScreenHeight, "Missile Commander!");
  SetTargetFPS(60);

  WELL512 rng(0);

  vector<Silo *> silos;
  vector<Missile *> missiles;
  vector<Explosion *> explosions;

  // Create terrain
  Rectangle ground_rec = {0, ScreenHeight - ground_dims.y, ground_dims.x,
                          ground_dims.y};

  // Create one silo for testing
  Silo *silo = new Silo;
  if (starting_missile_amount < 0)
    silo->missiles = 9999;
  silo->pos = {ScreenWidth / 2, ground_rec.y - silo_offset.y};
  silos.push_back(silo);

  float enemy_missile_spawn_timer = 0.;

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    enemy_missile_spawn_timer += dt;
    Vector2 mouse = GetMousePosition();

    // Get user input
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Missile *new_missile = new Missile;
      new_missile->dest = mouse;
      Vector2 mouse_diff =
          Vector2Subtract(mouse, silos[0]->pos); // TODO CHANGE THIS
      Vector2 norm = Vector2Normalize(mouse_diff);
      new_missile->ipos = Vector2Add(
          silos[0]->pos, Vector2Scale(norm, cannon_length)); // TODO CHANGE THIS
      new_missile->pos = new_missile->ipos;
      new_missile->vel = Vector2Scale(norm, missile_speed);
      missiles.push_back(new_missile);
    }

    // Physics update
    for (auto &missile : missiles) {
      Vector2 diff = Vector2Scale(missile->vel, dt);
      missile->pos = Vector2Add(missile->pos, diff);
    }

    for (auto &explosion : explosions) {
      explosion->rad += explosion->growth_dir * explosion->growth_speed * dt;
      if (explosion->rad >= explosion->max_rad) {
        explosion->growth_dir = -1.;
      }
    }

    // Collision checks
    for (int j = 0; j < explosions.size(); j++) {
      Explosion *explosion = explosions[j];
      for (int i = missiles.size() - 1; i >= 0; i--) {
        Missile *missile = missiles[i];
        Vector2 diff = Vector2Subtract(missile->pos, explosion->pos);
        float length = Vector2Length(diff);
        if (length <= explosion->rad) {
          Explosion *new_explosion = new Explosion;
          new_explosion->pos = missile->pos;
          new_explosion->rad = 0.;
          new_explosion->growth_dir = 1.;
          new_explosion->growth_speed = explosion_growth_speed;
          new_explosion->max_rad = explosion_max_rad;
          explosions.push_back(new_explosion);

          delete missile;
          missiles.erase(missiles.begin() + i);
        }
    }
    }

    // Condition checks
    if (enemy_missile_spawn_timer >= enemy_missile_spawn_time) {
      enemy_missile_spawn_timer = 0.;
      int count = rng.rand(3) + 1;
      for (int i = 0; i < count; i++) {
        Missile *new_missile = new Missile;
        new_missile->pos = {(float)rng.rand(ScreenWidth), 0};
        new_missile->ipos = new_missile->pos;
        new_missile->dest = silos[0]->pos; // TODO CHANGE THIS
        Vector2 diff = Vector2Subtract(new_missile->dest, new_missile->pos);
        Vector2 norm = Vector2Normalize(diff);
        new_missile->vel = Vector2Scale(norm, enemy_missile_speed);
        missiles.push_back(new_missile);
      }
    };

    for (int i = missiles.size() - 1; i >= 0; i--) {
      Missile *missile = missiles[i];
      Vector2 diff = Vector2Subtract(missile->dest, missile->pos);
      float length = Vector2Length(diff);
      if (length < missile_dest_tolerance) {
        Explosion *new_explosion = new Explosion;
        new_explosion->pos = missile->dest;
        new_explosion->rad = 0.;
        new_explosion->growth_dir = 1.;
        new_explosion->growth_speed = explosion_growth_speed;
        new_explosion->max_rad = explosion_max_rad;
        explosions.push_back(new_explosion);

        delete missile;
        missiles.erase(missiles.begin() + i);
      }
    }

    for (int i = explosions.size() - 1; i >= 0; i--) {
      Explosion *explosion = explosions[i];
      if (explosion->rad < 0.) {
        delete explosion;
        explosion = nullptr;
        explosions.erase(explosions.begin() + i);
      }
    }

    // @formatter off
    BeginDrawing();

    ClearBackground(BLACK);
    DrawFPS(10, 10);

    // Draw ground
    DrawRectangleRec(ground_rec, GREEN);

    // Draw missiles
    for (auto &missile : missiles) {
      Vector2 start = missile->ipos;
      Vector2 end = missile->pos;
      DrawLineV(start, end, GREEN);
    }

    // Draw explosions
    for (auto &explosion : explosions) {
      DrawCircleV(explosion->pos, explosion->rad, WHITE);
    }

    // Draw cannons
    for (auto &silo : silos) {
      Rectangle silo_rec = {silo->pos.x, silo->pos.y, silo_dims.x, silo_dims.y};
      DrawRectanglePro(silo_rec, silo_offset, 0., BROWN);
      Vector2 mouse_diff = Vector2Subtract(mouse, silo->pos);
      Vector2 norm = Vector2Normalize(mouse_diff);
      DrawLineEx(silo->pos,
                 Vector2Add(silo->pos, Vector2Scale(norm, cannon_length)),
                 cannon_thickness, WHITE);
    }

    // draw crosshair
    DrawLine(mouse.x - 5, mouse.y - 5, mouse.x + 5, mouse.y + 5, RED);
    DrawLine(mouse.x + 5, mouse.y - 5, mouse.x - 5, mouse.y + 5, RED);

    EndDrawing();
    // @formatter on
  }

  CloseWindow();

  return 0;
}