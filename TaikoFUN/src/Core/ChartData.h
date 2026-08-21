#pragma once

#include <vector>
#include "dxlib.h"
#include "Audio/SoundHandle.h"
// ノーツデータと譜面データ

enum class NoteType {
	Don,
	Katsu,
};

enum class JudgeType {
	GOOD,
	OK,
	BAD,
	MISS
};

struct Note
{

	const double bpm;				// ノーツの速度(流れる速さはこれに依存する)			SCROLLは譜面読み込み時に計算して結果をここに入れてもいいかも？例)BPM 185, scroll 0.5 : 185*0.5 = bpm
	const long long absTime;		// ノーツの絶対座標　(曲オフセットからの相対時間)	ノーツが流れてくる位置
	const NoteType type;				// ノーツタイプ (1=ドン, 2=カツ, 3=大ドン, 4=大カツ)

	bool isJudged = false;  // 既に判定を下かどうかを保持
	bool isMissed = false;
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



	void playSong(bool restart = false);
	void stopSong();
	void loadSong(const char* path);
	long long getSongCurrentTimeUs(bool applyOffset = false);
	long long songProgTime() { 
		_songProgTime = GetNowHiPerformanceCount() - songStartTime - totalPausedDuration;
		return _songProgTime; 
	}
	long long songProgTimefromOffset() {
		_songProgTimefromOffset = songProgTime() - offsetTime - totalPausedDuration;
		return _songProgTimefromOffset;
	}
};


// 描画に渡す判定結果のリストの要素のデータ
struct JudgeLog {
	JudgeType type;
	long long timeStamp; // 曲時間におけるタイムスタンプ
	bool isBig = false; //大音符
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

	long long judgeGOOD = 33000; // 良判定範囲時間(us)
	long long judgeOK = 66000; // 可判定範囲時間(us)
	long long judgeBAD = 100000; // 不可判定範囲時間(us)

	std::vector<JudgeLog> judgelogs;


	int good=0, ok=0, bad=0, miss=0; // ノーツの判定結果集計 good:良 ok:可 bad:不可 miss:叩かずにスルー
	int score = 0; // スコア
	int scoreGOOD = 200;	// 良判定のスコア (とりあえずハードコードで)
	int scoreOK = scoreGOOD / 2;
	int scoreBAD = 0;
	int scoreMISS = 0;

	void loadSong(const char* path, double bpm, double offset = 0.0);
	void playSong(bool restart = false);

	void Update();
	void Input();

	void nextNotes();
	long long noteRelativeTime(size_t noteIdx);	// 音源の再生位置からのノーツの相対座標を返す


private: 
	void updateMissNotes();	// ノーツが通り過ぎたことを更新する
	void judgeNote();
};

