#pragma once

#include <vector>
#include "dxlib.h"
#include "Audio/SoundHandle.h"
// ノーツデータと譜面データ

struct Note
{

	double bpm;				// ノーツの速度(流れる速さはこれに依存する)			SCROLLは譜面読み込み時に計算して結果をここに入れてもいいかも？例)BPM 185, scroll 0.5 : 185*0.5 = bpm
	long long absTime;		// ノーツの絶対座標　(曲オフセットからの相対時間)	ノーツが流れてくる位置
	int type;				// ノーツタイプ (1=ドン, 2=カツ, 3=大ドン, 4=大カツ)

	bool isJudged = false;  // 既に判定を下かどうかを保持

	// ToDo: 連打の実装、SCROLLなどの状態の実装
};

struct SongData
{

	long long songStartTime;	// 曲の絶対開始時間 (us)
	long long _songProgTime;		// 曲の経過時間(再生位置) (us)
	long long offsetTime;		// 曲のオフセット時間(譜面の開始位置) (us)
	long long _songProgTimefromOffset;	// 曲のオフセットからの経過時間 (us)
	long long lastPausedTimeStamp;	// 最後に一時停止した時間 (us)
	long long totalPausedDuration; // 一時停止した時間の合計 (us)

	SoundHandle songHandle;				// 曲のハンドル

	long long judgeGOOD = 3300; // 良判定範囲時間(us)
	long long judgeOK = 6600; // 可判定範囲時間(us)
	long long judgeBAD = 10000; // 不可判定範囲時間(us)



	void playSong(bool restart = false);
	void stopSong();
	void loadSong(const char* path);
	long long songProgTime() { 
		_songProgTime = GetNowHiPerformanceCount() - songStartTime - totalPausedDuration;
		return _songProgTime; 
	}
	long long songProgTimefromOffset() {
		_songProgTimefromOffset = songProgTime() - offsetTime - totalPausedDuration;
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
	size_t nextNoteIndex = 0; // 判定するノーツの位置

	int good=0, ok=0, bad=0; // ノーツの判定結果集計 good:良 ok:可 bad:不可
	int score = 0; // スコア

	void loadSong(const char* path, double bpm = 120.0, double offset = 0.0);
	void playSong(bool restart = false);


	void noteJudge();
	void nextNotes();

	void Update();

};

