#pragma once

#include "Scene/SceneContexts.h"
class AdjustJudgeOffset
{ 
public:

	AdjustJudgeOffset(GameContext* ctx): ctx_(ctx) {

	}

	void Input();
	void Draw();
	void Update();





private:

	const long long deltaUs = 100; // 1ms
	GameContext* ctx_;
	double offsetShowTimer_ = 0;

};

