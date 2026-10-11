#include "text.h"
#include "DxLib.h"

#include <string>

#include <stack>

void DrawFormatString2Right(int x, int y, unsigned int color, std::string str) {
	
	x -= GetDrawFormatStringWidth(str.c_str());
	DrawFormatString(x, y, color, str.c_str());
}

void DrawFormatString2Center( int x, int y, unsigned int color, std::string str ) {

	x -= GetDrawFormatStringWidth( str.c_str() )/2;
	DrawFormatString( x, y, color, str.c_str() );
}
