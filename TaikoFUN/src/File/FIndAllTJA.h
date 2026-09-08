#pragma once

#include <vector>
#include <string>

std::vector<std::string> FindAllTjaFiles(const std::string& songsRootPath);
std::string NormalizePath(const std::string& path);