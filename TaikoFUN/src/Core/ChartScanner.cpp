#include "ChartScanner.h"
#include <iostream>

std::vector<ChartScanner::ChartFile> ChartScanner::scanCharts(const std::filesystem::path& directoryPath) {
	std::vector<ChartFile> chartFiles;
	for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
		if (entry.is_regular_file() && entry.path().extension() == ".tja") {
			ChartFile chartFile;
			chartFile.Id = entry.path().stem().string();
			chartFile.ChartPath = entry.path();
			//chartFile.ThumbnailPath = entry.path().parent_path() / (entry.path().stem().string() + ".png");
			chartFiles.push_back(chartFile);
		}
	}
	std::cout << chartFiles.size() << " chart files found in " << directoryPath << std::endl;

	return chartFiles;
}
