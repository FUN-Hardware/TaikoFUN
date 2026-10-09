#pragma once

#include <vector>
#include "dxlib.h"
#include "Audio/SoundHandle.h"
#include "Audio/SongData.h"
#include "Core/Time.h"
#include "Note.h"
// ノーツデータと譜面データ





enum class CourseType {
	Easy,
	Normal,
	Hard,
	Oni,
	InnerOni,

};

struct gogoTime
{
	long long startTime;
	long long endTime;
};




// 描画に渡す判定結果のリストの要素のデータ


class ChartData
{

public:
	ChartData() = default;

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
	CourseType course = CourseType::Easy;
	double level = 0.0;
	std::vector<size_t> balloon;
	std::vector<gogoTime> gogoTimes;	// ゴーゴータイムの開始時間と終了時間のリスト

	long long judgeGOOD = 33000; // 良判定範囲時間(us)
	long long judgeOK = 66000; // 可判定範囲時間(us)
	long long judgeBAD = 100000; // 不可判定範囲時間(us)

	int scoreGOOD = 200;	// 良判定のスコア ベースとなるスコア OKはGOOD/2のスコア
	int scoreOK = scoreGOOD / 2;
	int scoreBAD = 0;
	int scoreMISS = 0;
	int scoreROLL = 100;
	int scoreBALLOONHIT = 10;
	int scoreBALLOONCLEARED = 1000;

	float GOGOSCOREMULTIPLIER = 1.2f;	// ゴーゴータイム中のスコア倍率

	void loadSong(const char* path);
	void loadSong(const char* path, double bpm, double offset = 0.0);
	void playSong(bool restart = false);

	void init(); // 初期化処理

	long long noteRelativeTime(size_t noteIdx);	// 音源の再生位置からのノーツの相対座標を返す

	

private: 


	const float rps = 20.0; // roll per sec
	const long long rollIntervalUs = static_cast<long long>(static_cast<float>(Time::TIME_US) / rps);
	




};

