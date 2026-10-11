#pragma once

#include "Chart/ChartData.h"

namespace Input {
	void Update();
	bool isKeyTriggered(int keyCode);
	bool isKeyDown(int keyCode);
	bool isAnyKeyDown();
	bool isAnyKeyTriggered();
	bool isNoteKeyTriggered(NoteType type);
}
