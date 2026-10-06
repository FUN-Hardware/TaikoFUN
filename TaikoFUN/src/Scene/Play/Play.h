#pragma once

#include "Chart/ChartData.h"
#include "Core/General.h"
#include "Scene/Scene.h"
#include "ChartPlayer.h"
#include "ChartDrawer.h"
#include "Scene/SceneContexts.h"

#include <vector>
#include <string>
#include <climits>
#include <memory>

class PlayScene : public Scene // シーンクラスを継承
{

	GameContext* ctx_;

	std::shared_ptr<ChartData> CD; // 譜面データの共有ポインタ ChartLoaderでロードし、PlaySceneで使用する。	
	std::unique_ptr<ChartPlayer> chartPlayer; // 演奏処理・判定処理を行うクラスのインスタンス。
	std::unique_ptr<ChartDrawer> chartDrawer; // 描画処理を行うクラスのインスタンス。「

	std::vector<std::string> tempTjaPath;
	std::string debug;

	size_t lastRollIdx = SIZE_MAX;
public:
	
	PlayScene(GameContext* ctx);

	void Init() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

};

