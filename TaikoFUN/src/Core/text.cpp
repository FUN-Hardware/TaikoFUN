#include "text.h"
#include "DxLib.h"

#include <string>

void DrawFormatString2Right(int x, int y, unsigned int color, std::string str) {
	
	x -= GetDrawFormatStringWidth(str.c_str());
	DrawFormatString(x, y, color, str.c_str());
}