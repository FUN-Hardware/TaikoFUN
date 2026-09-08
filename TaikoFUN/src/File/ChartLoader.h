#pragma once

#include "Core/ChartData.h"
#include <string>
#include <vector>





namespace ChartLoad {
	int load(const char* path, ChartData& cd, CourseType _course);

}