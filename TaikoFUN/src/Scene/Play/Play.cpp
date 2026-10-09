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
	effects_ = std::make_unique<PlayEffects>();
	chartPlayer = std::make_unique<ChartPlayer>(*CD);
	chartDrawer = std::make_unique<ChartDrawer>(*CD, *effects_);


}

void PlayScene::Init() {


}

void PlayScene::Draw() {

	chartDrawer->Draw(*chartPlayer);


}

void PlayScene::Update() {
	// 自動判定と手動発光は、F1 切り替え後の同じモードを参照する。
	if (Input::isKeyTriggered(KEY_INPUT_F1)) {
		chartPlayer->toggleAutoplay();
	}
	chartPlayer->Update();
	effects_->Update(*chartPlayer, Time::deltaSec());
	// リスタートによる演出初期化の後に、当該フレームの空打ちを追加する。
	Input();
	chartDrawer->Update(*chartPlayer);
}

void PlayScene::Input() {
	if (!chartPlayer->isAutoplay()) {
		if (Input::isKeyTriggered(KEY_INPUT_D)) {
			effects_->TriggerMiniDrumFlash(PlayEffects::DrumPart::LeftRim);
		}
		if (Input::isKeyTriggered(KEY_INPUT_F)) {
			effects_->TriggerMiniDrumFlash(PlayEffects::DrumPart::LeftFace);
		}
		if (Input::isKeyTriggered(KEY_INPUT_J)) {
			effects_->TriggerMiniDrumFlash(PlayEffects::DrumPart::RightFace);
		}
		if (Input::isKeyTriggered(KEY_INPUT_K)) {
			effects_->TriggerMiniDrumFlash(PlayEffects::DrumPart::RightRim);
		}
		if (Input::isNoteKeyTriggered(NoteType::Don)) {
			effects_->TriggerLaneFlash(NoteType::Don);
		}
		if (Input::isNoteKeyTriggered(NoteType::Katsu)) {
			effects_->TriggerLaneFlash(NoteType::Katsu);
		}
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


