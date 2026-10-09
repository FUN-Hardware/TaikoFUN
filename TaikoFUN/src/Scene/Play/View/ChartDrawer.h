#pragma once

#include "Chart/ChartData.h"
#include "BackgroundRenderer.h"
#include "NoteRenderer.h"
#include "PlayEffects.h"

#include <memory>

class ChartDrawer
{
public:
	ChartDrawer(ChartData& cd, PlayEffects& effects);
	void Init();
	void Update(const ChartPlayer& player);
	void Draw(const ChartPlayer& player);

private:
	ChartData& cd_;
	PlayEffects& effects_;
	std::unique_ptr<BackgroundRenderer> bg_;
	std::unique_ptr<NoteRenderer> nr_;
};
