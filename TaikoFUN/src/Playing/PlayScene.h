#pragma once

#include "Core/ChartData.h"
#include "Core/General.h"

#include <vector>
#include <string>
#include <climits>

#include "Core/ChartData.h"
class PlayScene
{
	
	ChartData CD;
	std::vector<std::string> tempTjaPath;
	std::string debug;

	size_t lastRollIdx = SIZE_MAX;
	// Drawing-only background state, calculated before Draw.
	struct PosterDrawObject {
		size_t sourceIndex;
		double centerX;
		double centerY;
		double angle;
	};
	std::vector<PosterDrawObject> posterDrawObjects;
	void UpdatePosterBackground(double elapsedSeconds);
public:
	
	PlayScene();


	void Update();
	void Draw();
	void Input();


};

