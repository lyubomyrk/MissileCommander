// MissileCommander.cpp : Defines the entry point for the application.
//

#include "MissileCommander.h"
#include "raylib.h"
#include "raymath.h"

using namespace std;

int main()
{
	const int ScreenWidth = 680;
	const int ScreenHeight = 480;

	InitWindow(ScreenWidth, ScreenHeight, "Missile Commander!");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawFPS(10, 10);
		EndDrawing();
	}

	CloseWindow();

	return 0;
}
