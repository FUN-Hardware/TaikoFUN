#pragma once

#include "Chart/ChartData.h"
#include "Core/General.h"
#include "Scene/Scene.h"
#include "Logic/ChartPlayer.h"
#include "View/ChartDrawer.h"
#include "View/PlayEffects.h"
#include "Scene/SceneContexts.h"
#include "Scene/Result/ResultLogic.h"

#include <vector>
#include <string>
#include <climits>
#include <memory>

class PlayScene : public Scene // シーンクラスを継承
{
public:

	PlayScene( GameContext* ctx );

	void Init() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

	void Input() override;

private:
	GameContext* ctx_;
	const PlayMode mode_;
	DemoLogic::Session demoSession_;
	int demoFont_ = -1;
	void ReturnFromDemo();

	std::shared_ptr<ChartData> CD; // 譜面データの共有ポインタ ChartLoaderでロードし、PlaySceneで使用する。	
	std::unique_ptr<PlayEffects> effects_;
	std::unique_ptr<ChartPlayer> chartPlayer; // 演奏処理・判定処理を行うクラスのインスタンス。
	std::unique_ptr<ChartDrawer> chartDrawer; // 描画処理を行うクラスのインスタンス。「

	std::vector<std::string> tempTjaPath;
	std::string debug;

	size_t lastRollIdx = SIZE_MAX;
	bool isStarted = false;


	void CaptureResult();
	ResultLogic::PlayEndTransition endTransition_;
	bool resultCaptured_ = false;
	unsigned long long lastGeneration_ = 0;
};

