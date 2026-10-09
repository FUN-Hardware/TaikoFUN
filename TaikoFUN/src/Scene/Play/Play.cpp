#include "DxLib.h"
#include "Play.h"
#include "Debug/FPS.h"

#include "Input/Input.h"
#include "Skin/SkinData.h"
#include "Core/General.h"
#include "Core/ChartScanner.h"
#include "File/ChartLoader.h"
#include "File/FindAllTJA.h"
#include "Core/text.h"
#include "Scene/SceneContexts.h"
#include "Scene/SceneID.h"

#include <unordered_set>
#include <memory>

namespace fs = std::filesystem;


PlayScene::PlayScene(GameContext* ctx) : ctx_(ctx){

	CD = ctx_->chartData;
	chartDrawer = std::make_unique<ChartDrawer>( *CD );
	chartPlayer = std::make_unique<ChartPlayer>( *CD );


}

void PlayScene::Init() {


}

void PlayScene::Draw() {

	chartDrawer->Draw(*chartPlayer);


}

void PlayScene::Update() {

	Input();
	chartPlayer->Update();
	chartDrawer->Update(*chartPlayer);


}


void PlayScene::Input() {

	if ( Input::isKeyTriggered( KEY_INPUT_F1 ) ) {
		chartPlayer->toggleAutoplay();
	}

}
// 演奏終了時の処理
void PlayScene::Finalize() {
	// PlayStatsからscoreなどのリザルト用データを抽出
	
	ResultCtx r;
	{
		r.title = CD->Title;
		r.subTitle = CD->subTitle;
		r.course = CD->course;
		r.level = CD->level;
		PlayStats ps = chartPlayer->getPlayStats();
		r.score = ps.score;
		r.combo = ps.combo;
		r.maxCombo = ps.maxCombo;
		r.goodCount = ps.goodCount;
		r.okCount = ps.okCount;
		r.badCount = ps.badCount;
		r.missCount = ps.missCount;
		r.rollHitCount = ps.rollHitCount;
		r.balloonHitCount = ps.balloonHitCount;
		r.balloonClearCount = ps.balloonClearCount;
		r.accuracy = ps.accuracy;
	}
	
	
	ctx_->resultData;
	RequestScene(SceneID::Result);

}


