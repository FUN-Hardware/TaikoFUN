#pragma once

#include "Core/ChartData.h"
#include <vector>
#include <string>
#include "Core/time.h"
#include "dxlib.h"



class Debug
{
	struct DebugInfo {
		std::string msg;
		double time = 3.0;
	};

	class Logs {
		std::vector<DebugInfo> logs;

	public:
		void add(std::string str, double time) {
			logs.push_back({ str, time });
		}

		void Update() {
			for (auto& log : logs) log.time -= Time::deltaSec();

			logs.erase(std::remove_if(logs.begin(), logs.end(), [](const DebugInfo& log) {
				return log.time <= 0.0;
				}), logs.end());
		}

		void Draw() {

			int x, y;
			GetWindowSize(&x, &y);

			int  strY = 16;
			int i = 0;
			for (const auto& log : logs) {
				DrawFormatString(10, y - strY * ++i, GetColor(255, 255, 255), "%s", log.msg.c_str());
			}
		}
	};

	long long songStartTime;
	long long songTime;
	int songHandle;

	long currentTime;

	bool songPlaying;

	bool calledUpdate = false;

	int soundHandles[10];

	


	ChartData chartData;
	Logs l;

public:

	Debug();
	~Debug();
	void Update();
	void Draw();
	void Input();
	void loadSong();








};

