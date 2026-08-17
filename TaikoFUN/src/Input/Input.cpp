#include "Input.h"
#include "DxLib.h"

#include <algorithm>
#include <iterator>


namespace {
	class Input {

		char allKeyStates[256];
		char allKeyStatesBuf[256];

	public:

		Input() {
			for (int i = 0; i < 256; i++) {
				allKeyStates[i] = 0;
				allKeyStatesBuf[i] = 0;
			}


		}

		void Update() {

			std::copy(std::begin(allKeyStates), std::end(allKeyStates), std::begin(allKeyStatesBuf));
			GetHitKeyStateAll(allKeyStates);

		}

		bool isKeyTriggered(int keyCode) {
			return allKeyStates[keyCode] && !allKeyStatesBuf[keyCode];
		}

		bool isKeyDown(int keyCode) {
			return allKeyStates[keyCode];
		}

	};


	Input g_state;
}


namespace Input {

	void Update() {
		g_state.Update();
	}

	bool isKeyTriggered(int keyCode) {
		return g_state.isKeyTriggered(keyCode);
	}

	bool isKeyDown(int keyCode) {
		return g_state.isKeyDown(keyCode);
	}

}