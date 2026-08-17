#pragma once

#include "Core/ChartData.h"
class Debug
{


	long songTime;
	int songHandle;

	long currentTime;

	bool songPlaying;

	bool calledUpdate = false;

	int soundHandles[10];

	ChartData chartData;

public:

	Debug();
	~Debug();
	void Update();
	void Draw();
	void Input();
	void loadSong();






};

