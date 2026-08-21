#pragma once

#include <string>
#include "Core/General.h"

class text
{

	std::string text;
	Vector2d pos;


};

void DrawFormatString2Right(int x, int y, unsigned int color, std::string str); // 右寄せ描画