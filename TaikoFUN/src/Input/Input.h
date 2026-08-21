#pragma once

#include "Core/ChartData.h"

namespace Input {
	void Update();
	bool isKeyTriggered(int keyCode);
	bool isKeyDown(int keyCode);
	bool isNoteKeyTriggered(NoteType type);
}