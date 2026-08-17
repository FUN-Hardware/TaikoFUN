#include "Fps.h"
#include "Dxlib.h"

// FPSを取得するクラス

namespace {
	class Fps {
		int fps = 0;
		int frameCount = 0;
		unsigned long long lastTime = 0;

	public:
		void Update() {


			unsigned long long curTime = GetNowHiPerformanceCount();

			if ((curTime - lastTime) > 1000000) {

				fps = frameCount;
				frameCount = 0;
				lastTime = curTime;

			}
			frameCount++;
		}

		int getFps() {
			return fps;
		}
	};

	Fps g_fps;
}

namespace FPS {
	void Update() {		// FPSを更新する
		g_fps.Update();
	}
	int getFps() { // FPSを取得する : FPS::getFps()で呼び出す
		return g_fps.getFps();
	}

}