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

  // LOAD FONT
  Font custom_font = LoadFont("../Resources/Font/Romulus.ttf");

  // PREPARE RNG
  WELL512 rng(0);

  // PREPARE GAME STATE
  GameScreen current_screen = GameScreen::Title;

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

  // Create cities
  vector<City *> cities;
  for (int i = 0; i < city_count / 2; i++) {
    Vector2 pos = {silos[0]->pos.x + (i + 1) * city_spacing,
                   ground_rec.y - city_offset.y};
    City *new_city = new City;
    new_city->pos = pos;
    cities.push_back(new_city);
  }
  for (int i = 0; i < city_count / 2; i++) {
    Vector2 pos = {silos[1]->pos.x + (i + 1) * city_spacing,
                   ground_rec.y - city_offset.y};
    City *new_city = new City;
    new_city->pos = pos;
    cities.push_back(new_city);
  }

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

  // Explosion color changes
  int explosion_red = 0;
  int explosion_red_dir = explosion_color_change;
  int explosion_green = 255;
  int explosion_green_dir = explosion_color_change;
  int explosion_blue = 0;
  int explosion_blue_dir = explosion_color_change;

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    Vector2 mouse = GetMousePosition();

    switch (current_screen) {
    case GameScreen::Title: {
      if (IsKeyPressed(KEY_SPACE)) {
        current_screen = GameScreen::Game;
      }

      BeginDrawing();
      ClearBackground(BLACK);
      float font_spacing = 1.;
      string title_message = "Missile Command!";
      float title_message_font_size = 60.;
      Vector2 title_message_size =
          MeasureTextEx(custom_font, title_message.c_str(),
                        title_message_font_size, font_spacing);
      DrawTextEx(
          custom_font, title_message.c_str(),
          {ScreenWidth / 2 - title_message_size.x / 2, ScreenHeight / 2 - 100},
          title_message_font_size, font_spacing, WHITE);
      string action_message = "Press SPACE to Play";
      float action_message_font_size = 20.;
      Vector2 action_message_size =
          MeasureTextEx(custom_font, action_message.c_str(),
                        action_message_font_size, font_spacing);
      DrawTextEx(
          custom_font, action_message.c_str(),
          {ScreenWidth / 2 - action_message_size.x / 2, ScreenHeight - 200},
          action_message_font_size, font_spacing, GREEN);
      EndDrawing();
    } break;
    case GameScreen::Game: {
      enemy_missile_spawn_timer += dt;

      // User input
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlaySound(launch_sound);
        Silo *silo = select_closest_silo(silos, mouse);
        create_missile(missiles, silo->pos, mouse, player_missile_flyweight);
      }

      // COLLISION CHECKS
      // Check missile v. explosion collision
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

      // Check explosion v. silo collision
      for (int i = silos.size() - 1; i >= 0; i--) {
        for (int j = 0; j < explosions.size(); j++) {
          Silo *silo = silos[i];
          Explosion *explosion = explosions[j];
          Vector2 diff = Vector2Subtract(silo->pos, explosion->pos);
          float length = Vector2Length(diff);
          if (length <= explosion->rad) {
            delete silo;
            silos.erase(silos.begin() + i);
            continue;
          }
        }
      }

      // Check explosion v. city collision
      for (int i = cities.size() - 1; i >= 0; i--) {
        for (int j = 0; j < explosions.size(); j++) {
          City *city = cities[i];
          Explosion *explosion = explosions[j];
          Vector2 diff = Vector2Subtract(city->pos, explosion->pos);
          float length = Vector2Length(diff);
          if (length <= explosion->rad) {
            delete city;
            cities.erase(cities.begin() + i);
            continue;
          }
        }
      }

      // STATUS
      // Check loss condition
      if (silos.size() == 0) {
        current_screen = GameScreen::Game_over;
      }

      // Spawn enemy missiles
      if (enemy_missile_spawn_timer >= enemy_missile_spawn_time) {
        enemy_missile_spawn_timer = 0.;
        int count = rng.well512rand(3) + 1;
        for (int i = 0; i < count; i++) {
          Vector2 ipos = {(float)rng.well512rand(ScreenWidth), 0};
          int city_or_silo = (int)rng.well512rand(4);
          Vector2 dest;
          if (city_or_silo == 0) {
            int silo_index = (int)rng.well512rand(silos.size());
            dest = silos[silo_index]->pos;
          } else {
            int city_index = (int)rng.well512rand(cities.size());
            dest = cities[city_index]->pos;
          }
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
        Vector2 start;
        Vector2 end;
        if (Vector2Distance(missile->pos, missile->ipos) >
            missile_trail_length) {
          Vector2 dir = Vector2Normalize(missile->vel);
          Vector2 trail = Vector2Scale(dir, missile_trail_length);
          Vector2 start = Vector2Subtract(missile->pos, trail);
          Vector2 end = missile->pos;
        } else {
          start = missile->ipos;
          end = missile->pos;
        }
        DrawLineV(start, end, missile->flyweight->color);
        DrawCircleV(missile->pos, missile_radius, missile->flyweight->color);
      }

      // Draw explosions
      for (auto &explosion : explosions) {
        explosion_red += explosion_red_dir;
        if (explosion_red < 0 || explosion_red > 255)
          explosion_red_dir *= -1;
        explosion_red = Clamp(explosion_red, 0., 255.);

        explosion_green += explosion_green_dir;
        if (explosion_green < 0 || explosion_green > 255)
          explosion_green_dir *= -1;
        explosion_green = Clamp(explosion_green, 0., 255.);

        explosion_blue += explosion_blue_dir;
        if (explosion_blue < 0 || explosion_blue > 255)
          explosion_blue_dir *= -1;
        explosion_blue = Clamp(explosion_blue, 0., 255.);

        Color color;
        color.a = 255;
        color.r = explosion_red;
        color.g = explosion_green;
        color.b = explosion_blue;
        DrawCircleV(explosion->pos, explosion->rad, color);
      }

      // Draw cannons
      for (auto &silo : silos) {
        Rectangle silo_rec = {silo->pos.x, silo->pos.y, silo_dims.x,
                              silo_dims.y};
        DrawRectanglePro(silo_rec, silo_offset, 0., BROWN);
        Vector2 mouse_diff = Vector2Subtract(mouse, silo->pos);
        Vector2 norm = Vector2Normalize(mouse_diff);
        DrawLineEx(silo->pos,
                   Vector2Add(silo->pos, Vector2Scale(norm, cannon_length)),
                   cannon_thickness, WHITE);
      }

      // Draw cities
      for (auto &city : cities) {
        Rectangle city_rec = {city->pos.x, city->pos.y, city_dims.x,
                              city_dims.y};
        DrawRectanglePro(city_rec, city_offset, 0., city_color);
      }

      // draw crosshair
      DrawLine(mouse.x - 5, mouse.y - 5, mouse.x + 5, mouse.y + 5, RED);
      DrawLine(mouse.x + 5, mouse.y - 5, mouse.x - 5, mouse.y + 5, RED);

      EndDrawing();

    } break;
    case GameScreen::Game_over: {
      BeginDrawing();
      float font_spacing = 1.;
      string title_message = "ALL IS LOST";
      float title_message_font_size = 100.;
      Vector2 title_message_size =
          MeasureTextEx(custom_font, title_message.c_str(),
                        title_message_font_size, font_spacing);
      DrawTextEx(
          custom_font, title_message.c_str(),
          {ScreenWidth / 2 - title_message_size.x / 2, ScreenHeight / 2 - 100},
          title_message_font_size, font_spacing, RED);
      EndDrawing();
    } break;
    default:
      break;
    }
  }

  CloseAudioDevice();
  CloseWindow();

  return 0;
}

Silo *select_closest_silo(vector<Silo *> &silos, Vector2 dest) {
  Silo *closest_silo;
  float shortest_dist = 9999;
  for (int i = 0; i < silos.size(); i++) {
    Silo *silo = silos[i];
    Vector2 diff = Vector2Subtract(dest, silo->pos);
    float dist = Vector2Length(diff);
    if (dist < shortest_dist) {
      closest_silo = silo;
      shortest_dist = dist;
    }
  }
  return closest_silo;
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

void run_game(vector<Silo *> &silos, vector<City *> &cities,
              vector<Missile *> &missiles, vector<Explosion *> &explosion,
              float dt) {}