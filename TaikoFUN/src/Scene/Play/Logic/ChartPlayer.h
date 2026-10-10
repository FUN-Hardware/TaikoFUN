#pragma once
#include "Chart/ChartData.h"
#include "Core/Time.h"
#include "JudgeEvent.h"
#include "PlayStats.h"

#include <algorithm>
#include <vector>


// ChartData.updateをこのクラスに移動します
class ChartPlayer
{ 
public:



	ChartPlayer( ChartData& data ): cd_( data ) { 
	};



	void Update();
	// 済み
	void loadSong( const char* path );
	void loadSong( const char* path, double bpm, double offset = 0.0 );
	void playSong( bool restart = false );
	// 済み
	void init(); // 初期化処理
	// 済み

	long long noteRelativeTime( size_t noteIdx );	// 音源の再生位置からのノーツの相対座標を返す
	// 済み
	PlayStats getPlayStats() const { return stats_; } // 描画などで使用するためのゲッター

	bool isAutoplay() const { return autoPlay; }
	const std::vector<JudgeEvent>& getJudgeEvents() const { return events_; }
	size_t getPlaybackGeneration() const { return playbackGeneration_; }
	double getPlaybackElapsedSec() const {
		return cd_.songData.playing
			? (std::max)(0.0, (nowSongTime + cd_.songData.offsetTime) / 1000000.0) : 0.0;
	}

	bool isGogoTime() const;

	void toggleAutoplay() { 
		autoPlay = !autoPlay;
	}

	bool isFinished = false;

private:

	ChartData& cd_; // このオブジェクトが生成される際にGameContextから譜面データを受け取る
	PlayStats stats_; // 判定結果の集計などを保持するオブジェクト
	
	bool autoPlay = false; // オートプレイ
	size_t nextNoteIndex = 0; // 判定するノーツの位置
	long long nowSongTime = 0; //　cd.songData.getSongCurrentTimeUs(true)のキャッシュ

	std::vector<JudgeEvent> events_; // このフレームの判定。描画側は読み取り専用。
	size_t playbackGeneration_ = 0;

	const float GOGOSCOREMULTIPLIER = 1.2f;	// ゴーゴータイム中のスコア倍率

	const long long judgeGOOD = 33000; // 良判定範囲時間(us)
	const long long judgeOK = 66000; // 可判定範囲時間(us)
	const long long judgeBAD = 100000; // 不可判定範囲時間(us)

	const int scoreGOOD = 200;	// 良判定のスコア ベースとなるスコア OKはGOOD/2のスコア
	const int scoreOK = scoreGOOD / 2;
	const int scoreBAD = 0;
	const int scoreMISS = 0;
	const int scoreROLL = 100;
	const int scoreBALLOONHIT = 10;
	const int scoreBALLOONCLEARED = 1000;

	const float rps = 20.0; // roll per sec
	const long long rollIntervalUs = static_cast<long long>(static_cast<float>(Time::TIME_US) / rps);


	size_t lastRollIdx = SIZE_MAX;
	size_t gogoIndex = SIZE_MAX;
	long long lastRollHitUs = 0;


	void input();
	// 済み
	void nextNotes();
	// 済み
	void updateMissNotes();	// ノーツが通り過ぎたことを更新する
	// 済み
	void judgeNote();
	// 済み
	void autoplayHitNote();	// オートプレイの処理
	// 済み
	void applyNoteJudge( Note& targetNote, JudgeType judgeType );
	// 済み
	//void updateGogoTime();	// ゴーゴータイムの更新

};

/*
	SongData songData;
	std::vector<Note> notes; // マイクロ秒単位
	size_t nextNoteIndex = 0; // 判定するノーツの位置
	bool autoPlay = true; // オートプレイ

	std::string tjaPath = "";

	std::string Title = "";
	std::string subTitle = "";
	std::string songPath = "";
	double offset = 0.0;
	double bpm = 120.0;
	double demoStart = 0.0;

	long long nowTime;
	long long nowSongTime;
	CourseType course = CourseType::Easy;
	double level = 0.0;
	std::vector<size_t> balloon;
	std::vector<gogoTime> gogoTimes;	// ゴーゴータイムの開始時間と終了時間のリスト
	size_t gogoIndex = 0;	// 現在のゴーゴータイムのインデックス
	bool isGogoTime = false;	// 現在ゴーゴータイム中かどうか

	long long judgeGOOD = 33000; // 良判定範囲時間(us)
	long long judgeOK = 66000; // 可判定範囲時間(us)
	long long judgeBAD = 100000; // 不可判定範囲時間(us)

	std::vector<JudgeLog> judgelogs;
	size_t lastRollIdx = SIZE_MAX;


	int good=0, ok=0, bad=0, miss=0; // ノーツの判定結果集計 good:良 ok:可 bad:不可 miss:叩かずにスルー

	int score = 0; // 現在のスコア


	int scoreGOOD = 200;	// 良判定のスコア ベースとなるスコア OKはGOOD/2のスコア
	int scoreOK = scoreGOOD / 2;
	int scoreBAD = 0;
	int scoreMISS = 0;
	int scoreROLL = 100;
	int scoreBALLOONHIT = 10;
	int scoreBALLOONCLEARED = 1000;

	float GOGOSCOREMULTIPLIER = 1.2f;	// ゴーゴータイム中のスコア倍率

	int combo = 0;
	int MAXcombo = 0;

	void loadSong(const char* path);
	void loadSong(const char* path, double bpm, double offset = 0.0);
	void playSong(bool restart = false);

	void init(); // 初期化処理

	void Update();
	void Input();

	void nextNotes();
	long long noteRelativeTime(size_t noteIdx);	// 音源の再生位置からのノーツの相対座標を返す



private:


	const float rps = 20.0; // roll per sec
	const long long rollIntervalUs = static_cast<long long>(static_cast<float>(Time::TIME_US) / rps);


	long long lastRollHitUs = 0;


	void updateMissNotes();	// ノーツが通り過ぎたことを更新する
	void judgeNote();
	void autoplayHitNote();	// オートプレイの処理

	void applyNoteJudge(Note& targetNote, JudgeType judgeType);
	void updateGogoTime();	// ゴーゴータイムの更新
*/