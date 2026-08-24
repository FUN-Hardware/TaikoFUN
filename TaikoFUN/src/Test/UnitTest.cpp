#include "pch.h"
#include <iostream>
#include "CppUnitTest.h"
#include "Core/ChartScanner.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			ChartScanner scanner;
			scanner.scanCharts("C:\\Users\\dotdo\\source\\repos\\FUN-Hardware\\TaikoFUN\\TaikoFUN\\Resource\\Debug\\シャイニングスター/");
		}
	};
}