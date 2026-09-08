#pragma once
#include <vector>
#include <filesystem>
#include <string>

struct ChartScanner {

	//読み込むCSVファイルのテーブル一覧を定義した構造体

	typedef struct ChartFile {
		std::string Id;
		std::filesystem::path ChartPath;
		//std::filesystem::path ThumbnailPath;

	} ChartFile;


	
	std::vector<ChartFile> scanCharts(const std::filesystem::path& directoryPath);
	
};

