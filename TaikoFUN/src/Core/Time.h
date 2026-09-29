#pragma once


namespace Time {
	constexpr long long TIME_US = 1000000;



	void Update();

	double deltaUs();
	double deltaSec();
	long long lastTime();
	long long nowTime();

};