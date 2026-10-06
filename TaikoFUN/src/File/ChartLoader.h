#pragma once

#include "Chart/ChartData.h"
#include <string>
#include <vector>





namespace ChartLoad {
	int load(const char* path, ChartData& cd, CourseType _course);

}