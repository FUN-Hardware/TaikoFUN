#pragma once
#include "Scene/Play/Logic/ChartPlayer.h"

class ChartPlayer; // 前方宣言

class BackgroundRenderer
{
public:

	BackgroundRenderer() = default;

	void Init();
	void Draw(const ChartPlayer& cp_);
	void Update();


private:

};
