#pragma once

#include <vector>
#include "dxlib.h"

// ノーツデータと譜面データ

struct Note
{

	// ノーツの絶対時間とノーツタイプ(1=ドン, 2=カツ, 3=大ドン, 4=大カツ)
	long long absTime;
	int type;

	// ToDo: 連打の実装、SCROLLなどの状態の実装
};

struct SongData
{

	long long songStartTime;	// 曲の絶対開始時間
	long long _songProgTime;		// 曲の経過時間(再生位置)
	long long offsetTime;		// 曲のオフセット時間(譜面の開始位置)
	long long _songProgTimefromOffset;	// 曲のオフセットからの経過時間
	int songHandle;				// 曲のハンドル

	void playSong();
	void loadSong(const char* path);
	long long songProgTime() { 
		_songProgTime = GetNowHiPerformanceCount() - songStartTime;
		return _songProgTime; 
	}
	long long songProgTimefromOffset() {
		_songProgTimefromOffset = songProgTime() - offsetTime;
		return _songProgTimefromOffset;
	}
};


class ChartData
{

public:
	SongData songData;
	double bpm;
	std::vector<Note> notes;


	void loadSong(const char* path, double bpm = 120.0);
	void playsong();
};

