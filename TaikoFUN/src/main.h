#pragma once




enum class GameState
{
	Title,
	Playing,
	Result,
	Debug,
	Null
};

void Update(GameState state);
void Draw(GameState state);
