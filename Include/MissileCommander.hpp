#pragma once
#include "WELL512.hpp"
#include "raylib.h"
#include "raymath.h"
#include <string>
#include <vector>

using namespace std;

// Game screen
enum class GameScreen { Title, Game, Game_over };

struct Silo {
  Vector2 pos;
};

struct City {
  Vector2 pos;
};

// Generic representation of every missile in game
enum class MissileType { Player, Enemy };

struct MissileFlyweight {
  MissileType type;
  Color color;
  float speed;
};

struct Missile {
  Vector2 ipos;
  Vector2 pos;
  Vector2 vel;
  Vector2 dest;

  MissileFlyweight *flyweight;
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
const int ScreenHeight = 600;

const Vector2 ground_dims = {ScreenWidth, 50.};

// Silo
const int silo_count = 3;
const int starting_missile_amount = -1.;
const Vector2 silo_dims = {50., 25.};
const Vector2 silo_offset = {silo_dims.x / 2, silo_dims.y / 2};
const float silo_screen_offset = 100.;
const float cannon_length = 30;
const float cannon_thickness = 5;

// City
const int city_count = 6;
const float city_spacing = 75.;
const Vector2 city_dims = {25., 25.};
const Vector2 city_offset = {city_dims.x / 2, city_dims.y / 2};
const Color city_color = MAGENTA;

// Missiles
const float missile_radius = 2.5;
const float missile_trail_length = 150.;
const float missile_dest_tolerance = 5.;
const float player_missile_speed = 250;
const float enemy_missile_speed = 50.;
const float enemy_missile_spawn_time = 2.5;

// Explosions
const float explosion_growth_speed = 50.;
const float explosion_max_rad = 40.;
const float explosion_color_change = 40.;

Silo *select_closest_silo(vector<Silo *> &silos, Vector2 dest);
void create_missile(vector<Missile *> &missiles, Vector2 ipos, Vector2 dest,
                    MissileFlyweight *flyweight);
void create_explosion(vector<Explosion *> &explosions, Vector2 pos);

void run_game(vector<Silo *> &silos, vector<City *> &cities,
              vector<Missile *> &missiles, vector<Explosion *> &explosion,
              float dt);