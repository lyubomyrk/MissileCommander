// MissileCommander.cpp : Defines the entry point for the application.

#include "MissileCommander.hpp"

using namespace std;

int main() {
  InitWindow(ScreenWidth, ScreenHeight, "Missile Commander!");
  SetTargetFPS(60);

  InitAudioDevice();

  // LOAD ASSETS
  Sound launch_sound = LoadSound("../Assets/Sound/launch.ogg");
  Sound explosion_sound = LoadSound("../Assets/Sound/explosion.ogg");

  // PREPARE RNG
  WELL512 rng(0);

  // PREPARE GAME STATE
  // Create terrain
  Rectangle ground_rec = {0, ScreenHeight - ground_dims.y, ground_dims.x,
                          ground_dims.y};

  // Create silos
  vector<Silo *> silos;
  Silo *left_silo = new Silo;
  left_silo->pos = {silo_screen_offset, ground_rec.y - silo_offset.y};
  silos.push_back(left_silo);

  Silo *middle_silo = new Silo;
  middle_silo->pos = {ScreenWidth / 2, ground_rec.y - silo_offset.y};
  silos.push_back(middle_silo);

  Silo *right_silo = new Silo;
  right_silo->pos = {ScreenWidth - silo_screen_offset,
                     ground_rec.y - silo_offset.y};
  silos.push_back(right_silo);

  // Prepare missiles
  vector<Missile *> missiles;
  MissileFlyweight *player_missile_flyweight = new MissileFlyweight;
  player_missile_flyweight->type = MissileType::Player;
  player_missile_flyweight->color = GREEN;
  player_missile_flyweight->speed = player_missile_speed;
  MissileFlyweight *enemy_missile_flyweight = new MissileFlyweight;
  enemy_missile_flyweight->type = MissileType::Enemy;
  enemy_missile_flyweight->color = RED;
  enemy_missile_flyweight->speed = enemy_missile_speed;

  // Prepare explosions
  vector<Explosion *> explosions;

  // Create game timers
  float enemy_missile_spawn_timer = 0.;

  // GAME LOOP
  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    enemy_missile_spawn_timer += dt;
    Vector2 mouse = GetMousePosition();

    // User input
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      PlaySound(launch_sound);
      int silo_index = mouse.x / (ScreenWidth / silo_count);
      create_missile(missiles, silos[silo_index]->pos, mouse,
                     player_missile_flyweight);
    }

    // COLLISION CHECKS
    // Check missile/explosion collision
    for (int j = 0; j < explosions.size(); j++) {
      Explosion *explosion = explosions[j];
      for (int i = missiles.size() - 1; i >= 0; i--) {
        Missile *missile = missiles[i];
        Vector2 diff = Vector2Subtract(missile->pos, explosion->pos);
        float length = Vector2Length(diff);
        if (length <= explosion->rad) {
          create_explosion(explosions, missile->pos);
          delete missile;
          missiles.erase(missiles.begin() + i);
        }
      }
    }

    // STATUS
    // Spawn enemy missiles
    if (enemy_missile_spawn_timer >= enemy_missile_spawn_time) {
      enemy_missile_spawn_timer = 0.;
      int count = rng.rand(3) + 1;
      for (int i = 0; i < count; i++) {
        Vector2 ipos = {(float)rng.rand(ScreenWidth), 0};
        int silo_index = (int)rng.rand(silos.size());
        Vector2 dest = silos[silo_index]->pos;
        create_missile(missiles, ipos, dest, enemy_missile_flyweight);
      }
    };

    // Check missile destination reached
    for (int i = missiles.size() - 1; i >= 0; i--) {
      Missile *missile = missiles[i];
      Vector2 diff = Vector2Subtract(missile->dest, missile->pos);
      float length = Vector2Length(diff);
      if (length < missile_dest_tolerance) {
        PlaySound(explosion_sound);
        create_explosion(explosions, missile->dest);
        delete missile;
        missiles.erase(missiles.begin() + i);
      }
    }

    // Remove dead explosions
    for (int i = explosions.size() - 1; i >= 0; i--) {
      Explosion *explosion = explosions[i];
      if (explosion->rad < 0.) {
        delete explosion;
        explosions.erase(explosions.begin() + i);
      }
    }

    // MOVE
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
      DrawLineV(start, end, missile->flyweight->color);
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

  CloseAudioDevice();
  CloseWindow();

  return 0;
}

void create_missile(vector<Missile *> &missiles, Vector2 ipos, Vector2 dest,
                    MissileFlyweight *flyweight) {
  Missile *new_missile = new Missile;
  new_missile->dest = dest;
  // CHANGE THIS TO A FUNCTION select_silo(Vector2 dest) LATER
  Vector2 dest_diff = Vector2Subtract(dest, ipos);
  Vector2 norm = Vector2Normalize(dest_diff);
  new_missile->ipos = Vector2Add(ipos, Vector2Scale(norm, cannon_length));
  new_missile->pos = new_missile->ipos;
  new_missile->vel = Vector2Scale(norm, flyweight->speed);
  new_missile->flyweight = flyweight;
  missiles.push_back(new_missile);
}

void create_explosion(vector<Explosion *> &explosions, Vector2 pos) {
  Explosion *new_explosion = new Explosion;
  new_explosion->pos = pos;
  new_explosion->rad = 0.;
  new_explosion->growth_dir = 1.;
  new_explosion->growth_speed = explosion_growth_speed;
  new_explosion->max_rad = explosion_max_rad;
  explosions.push_back(new_explosion);
}