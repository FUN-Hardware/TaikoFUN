#pragma once

#include <string>
#include <vector>

class General
{
};


struct Vector2d{
	double x;
	double y;
};

struct Vector2i {
	int x;
	int	 y;
};

/*
std::string substrBack(std::string str, size_t pos, size_t len) {
	const size_t strLen = str.length();

	return str.substr(strLen - pos, len);
}*/
namespace string_util {
	std::string trimWhitespace(const std::string& str);

	std::vector<std::string> split(const std::string& str, const char delimiter = ',');
}
