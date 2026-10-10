#pragma once

// 曲選択画面で使用するデータの塊
#include <string>
#include <vector>
#include "Chart/CourseType.h"
#include "Audio/SongData.h"



struct SongItemData
{

	std::string tjaPath = "";
	std::string title = "";
	std::string songPath;
	long long DemoStartAt = 0;
	float levels[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
	bool Courses[5] = { 0, 0, 0, 0, 0 };
};