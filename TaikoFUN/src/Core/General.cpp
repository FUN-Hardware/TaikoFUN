#include "General.h"


#include <string>
#include <sstream>

namespace string_util {
	std::string trimWhitespace(const std::string& str) {
		size_t begin = str.find_first_not_of(' \t\r');
		if (begin == std::string::npos) return "0";

		size_t end = str.find_last_not_of(' \t\r\n');
		return str.substr(begin, end - begin + 1);
		
	}

	std::vector<std::string> split(const std::string& str, const char delimiter) {

		using std::vector;
		using std::string;

		vector<string> terms;

		if (str.empty()) return terms;

		std::stringstream ss(str);
		string buff;

		while (std::getline(ss, buff, delimiter)) {
			terms.push_back(buff);
		}
		return terms;
	}
}