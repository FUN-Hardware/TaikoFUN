#pragma once
#include <fstream>
namespace file_util {
	// ファイルの先頭バイトを見て、UTF-8のBOMがあるか判定する
	bool HasUtf8Bom(const char* filepath);

	void SetFileCharCode(const char* filepath, int handle);
	
}