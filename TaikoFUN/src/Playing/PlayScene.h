#pragma once

#include "Core/ChartData.h"
#include "Core/General.h"

#include <vector>
#include <string>

class PlayScene
{
	
	ChartData CD;
	std::vector<std::string> tempTjaPath;
	std::string debug;
public:
	
	PlayScene();


	void Update();
	void Draw();
	void Input();


};

