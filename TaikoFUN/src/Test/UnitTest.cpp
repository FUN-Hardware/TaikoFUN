#include "pch.h"
#include "CppUnitTest.h"
#include "Core/ChartScanner.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace
{
	class TemporaryDirectory
	{
	public:
		TemporaryDirectory()
		{
			const auto uniqueId = std::chrono::steady_clock::now()
				.time_since_epoch()
				.count();
			path_ = std::filesystem::temp_directory_path()
				/ ("TaikoFUN_ChartScanner_" + std::to_string(uniqueId));
			std::filesystem::create_directories(path_);
		}

		~TemporaryDirectory()
		{
			std::error_code error;
			std::filesystem::remove_all(path_, error);
		}

		const std::filesystem::path& path() const
		{
			return path_;
		}

	private:
		std::filesystem::path path_;
	};

	void CreateEmptyFile(const std::filesystem::path& path)
	{
		std::ofstream file(path);
	}
}

namespace UnitTest
{
	TEST_CLASS(ChartScannerTest)
	{
	public:
		TEST_METHOD(ScanChartsReturnsOnlyTjaFiles)
		{
			TemporaryDirectory testDirectory;
			const auto chartPath = testDirectory.path() / "sample.tja";

			CreateEmptyFile(chartPath);
			CreateEmptyFile(testDirectory.path() / "readme.txt");

			ChartScanner scanner;
			const auto chartFiles = scanner.scanCharts(testDirectory.path());

			Assert::AreEqual<std::size_t>(1, chartFiles.size());
			Assert::IsTrue(chartFiles.front().ChartPath == chartPath);
			Assert::AreEqual(std::string("sample"), chartFiles.front().Id);
		}
	};
}
