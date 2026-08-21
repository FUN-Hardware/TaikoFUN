#include "Input.h"
#include "DxLib.h"
#include "Core/ChartData.h"

#include <algorithm>
#include <iterator>
#include <unordered_map>


namespace {
	class Input {

		char allKeyStates[256];
		char allKeyStatesBuf[256];

		std::unordered_map<NoteType, bool> triggeredInput;	// ドンとカツの入力のみを保持
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

			
			triggeredInput[NoteType::Don] = (isKeyTriggered(KEY_INPUT_F) || isKeyTriggered(KEY_INPUT_J));
			triggeredInput[NoteType::Katsu] = (isKeyTriggered(KEY_INPUT_D) || isKeyTriggered(KEY_INPUT_K));

		}

		bool isKeyTriggered(int keyCode) {
			return allKeyStates[keyCode] && !allKeyStatesBuf[keyCode];
		}

		bool isKeyDown(int keyCode) {
			return allKeyStates[keyCode];
		}

		bool isNoteKeyTriggered(NoteType type) {
			return triggeredInput[type];
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

	bool isNoteKeyTriggered(NoteType type) {
		return g_state.isNoteKeyTriggered(type);
	}
}