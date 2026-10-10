#include "DxLib.h"
#include "Play.h"
#include "Debug/FPS.h"

#include "Dxlib.h"
#include "Input/Input.h"
#include "Skin/SkinData.h"
#include "Core/General.h"
#include "Core/ChartScanner.h"
#include "File/ChartLoader.h"
#include "File/FindAllTJA.h"
#include "Core/text.h"
#include "Scene/SceneContexts.h"
#include "Scene/SceneID.h"
#include "Debug/FPS.h"

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


	DrawFormatString2( 0,
					   0,
					   BACKGROUND_GREEN,
					   BLACKNESS,
					   "FPS: %d", FPS::getFps());

	// ループの先頭で
	static long long prev = GetNowHiPerformanceCount();
	long long now = GetNowHiPerformanceCount();
	double ms = (now - prev) / 1000.0;  prev = now;
	static double maxMs = 0;  if ( ms > maxMs ) maxMs = ms;
	DrawFormatString( 10, 50, GetColor( 255, 255, 255 ), "frame=%.1fms max=%.1fms", ms, maxMs );


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

	if ( chartPlayer->isFinished ) {
		sceneChangeDelay -= Time::deltaUs();
		if ( sceneChangeDelay < 0 ) {
			RequestScene( SceneID::SongSelect );
		}
	}

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
	
	chartPlayer->stopSong();
	ctx_->resultData;
	

}


