#pragma once

#include <vector>
#include "dxlib.h"
#include "Audio/SoundHandle.h"
// ノーツデータと譜面データ

struct Note
{

	// ノーツの絶対時間とノーツタイプ(1=ドン, 2=カツ, 3=大ドン, 4=大カツ)
	double bpm;
	long long absTime;
	int type;

	// ToDo: 連打の実装、SCROLLなどの状態の実装
};

struct SongData
{

	long long songStartTime;	// 曲の絶対開始時間 (us)
	long long _songProgTime;		// 曲の経過時間(再生位置) (us)
	long long offsetTime;		// 曲のオフセット時間(譜面の開始位置) (us)
	long long _songProgTimefromOffset;	// 曲のオフセットからの経過時間 (us)
	SoundHandle songHandle;				// 曲のハンドル

	long long judgeGOOD = 3300; // 良判定範囲時間(us)
	long long judgeOK = 6600; // 可判定範囲時間(us)
	long long judgeBAD = 10000; // 不可判定範囲時間(us)



	void playSong(bool restart = false);
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
	ChartData() = default;
	ChartData(const char* path) {
		songData.loadSong(path);
	}


	SongData songData;
	double bpm;
	std::vector<Note> notes; // マイクロ秒単位


	void loadSong(const char* path, double bpm = 120.0, double offset = 0.0);
	void playsong();
};

