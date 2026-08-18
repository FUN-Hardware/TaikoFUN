#include "Time.h"
#include "dxlib.h"


namespace {
	long long g_nowTimeMs = 0;
	long long g_dtMs = 0;
	double g_dtSec = 0;
	long long g_lastTime = 0;
}


namespace Time {

	void Update() {
		g_nowTimeMs = GetNowHiPerformanceCount();
		g_dtMs = g_nowTimeMs - g_lastTime;
		g_dtSec = g_dtMs / 100000.0;
		g_lastTime = g_nowTimeMs;
	}
	double deltaMs() { return g_dtMs; }
	double deltaSec() { return g_dtSec; }
	long long lastTime() { return g_lastTime; }
	long long nowTime() { return g_nowTimeMs; }
}