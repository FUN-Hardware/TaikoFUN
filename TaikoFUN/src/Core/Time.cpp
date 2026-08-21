#include "Time.h"
#include "dxlib.h"


namespace {
	long long g_nowTimeUs = 0;
	long long g_dtUs = 0;
	double g_dtSec = 0;
	long long g_lastTime = 0;
}


namespace Time {

	void Update() {
		g_nowTimeUs = GetNowHiPerformanceCount();
		g_dtUs = g_nowTimeUs - g_lastTime;
		g_dtSec = g_dtUs / 1000000.0;
		g_lastTime = g_nowTimeUs;
	}
	double deltaUs() { return g_dtUs; }
	double deltaSec() { return g_dtSec; }
	long long lastTime() { return g_lastTime; }
	long long nowTime() { return g_nowTimeUs; }
}