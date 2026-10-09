#pragma once

#include "Chart/ChartData.h"
#include <memory>
#include <string>
#include <vector>





namespace ChartLoad {
	std::shared_ptr<ChartData> load(const char* path, CourseType _course);

}