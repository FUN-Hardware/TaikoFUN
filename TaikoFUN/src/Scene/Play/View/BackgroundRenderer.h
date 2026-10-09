#pragma once

#include <cstddef>
#include <vector>

class ChartPlayer;

class BackgroundRenderer
{
public:
	BackgroundRenderer();
	void Init();
	void Draw(const ChartPlayer& player) const;
	void Update(const ChartPlayer& player);

private:
	struct PosterDrawObject {
		size_t sourceIndex;
		double centerX, centerY, angle;
	};
	std::vector<PosterDrawObject> posterDrawObjects;
	void UpdatePosterBackground(double elapsedSeconds);
};
