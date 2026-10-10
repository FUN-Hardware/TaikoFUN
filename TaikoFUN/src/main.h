#pragma once

class SceneManager;


enum class GameState
{
	Title,
	Playing,
	Result,
	Debug,
	Null
};

void Update( SceneManager& sm );
void Draw(SceneManager& sm);
