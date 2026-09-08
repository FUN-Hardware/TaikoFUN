#include "FIndAllTJA.h"

#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

std::vector<std::string> FindAllTjaFiles(const std::string& songsRootPath) {
    std::vector<std::string> result;

    for (const auto& entry : fs::recursive_directory_iterator(songsRootPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".tja") {
            if (false) {
                std::u8string path_u8 = entry.path().u8string();
                std::string utf8_path(reinterpret_cast<const char*>(path_u8.data()), path_u8.size());
                result.push_back(NormalizePath(utf8_path));
            }
            else {
                std::string path = entry.path().string();
                result.push_back(path);
            }
        }
    }

    return result;
}

std::string NormalizePath(const std::string& path) {
	std::string result = path;
	std::replace(result.begin(), result.end(), '\\', '/'); // \ を全部 / に置き換える
	return result;
}