#pragma once
#include "Chart/ChartData.h"

#include <memory>
// シーンのコンテキスト

struct ResultCtx
{
	int score;
};


// 今後の設計でシーンマネージャからそれぞれのシーンにデータが渡されます
struct GameContext
{
	// ゲームの状態を保持する変数やオブジェクト

	std::shared_ptr<ChartData> chartData; // 譜面データの共有ポインタ ChartLoaderでロードし、PlaySceneで使用する。



};