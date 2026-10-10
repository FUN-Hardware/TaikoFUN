#pragma once
#include "Chart/ChartData.h"
#include "SongSelect/SelectedCourse.h"
#include <memory>
#include <string>
// シーンのコンテキスト

struct ResultCtx
{
	std::string title = "";
	std::string subTitle = "";
	CourseType course = CourseType::Oni;
	float level = 1.0;
	int score = 0;
	int combo = 0;
	int maxCombo = 0;
	int goodCount = 0;
	int okCount = 0;
	int badCount = 0;
	int missCount = 0;
	int rollHitCount = 0;
	int balloonHitCount = 0;
	int balloonClearCount = 0;
	float accuracy = 0.0f;
};


// 今後の設計でシーンマネージャからそれぞれのシーンにデータが渡されます
struct GameContext
{
	// ゲームの状態を保持する変数やオブジェクト

	std::shared_ptr<ChartData> chartData; // 譜面データの共有ポインタ ChartLoaderでロードし、PlaySceneで使用する。
	std::shared_ptr<ResultCtx> resultData; // リザルトデータの集まり。PlayScene終了時にPlayStatsから代入される。 使用箇所はリザルトシーン
	SelectedCourse selectedCourse;

};