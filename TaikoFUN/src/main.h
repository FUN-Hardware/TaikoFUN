#pragma once

void Update();
void Draw();

enum class GameState
{
	Title,
	Playing,
	Result,
	Debug
};
