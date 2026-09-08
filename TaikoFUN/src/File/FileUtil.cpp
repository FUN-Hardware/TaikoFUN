#include "FileUtil.h"

#include <fstream>
#include <cassert>

namespace file_util {
	std::vector<std::string> getAllLines(const std::string& filepath) {
		std::vector<std::string> result;
		std::ifstream file(filepath);
		std::string line;
		bool isFirstLine = true;

		if (!file.is_open()) {
			assert(false && "ファイルが見つかりません");
			return result;
		}

		while (std::getline(file, line)) {
			if (isFirstLine) {
				// BOM(EF BB BF)が先頭にあれば、取り除く
				if (line.size() >= 3 &&
					(unsigned char)line[0] == 0xEF &&
					(unsigned char)line[1] == 0xBB &&
					(unsigned char)line[2] == 0xBF) {
					line = line.substr(3); // 先頭3バイトを除去する
				}
				isFirstLine = false;
			}
			result.push_back(line);
		}

		return result;
	}
}