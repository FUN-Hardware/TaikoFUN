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
	chartPlayer = std::make_unique<ChartPlayer>(*CD, ctx_->judgeOffsetUs);
	chartDrawer = std::make_unique<ChartDrawer>(*CD, *effects_);


}

void PlayScene::Init() {
	ctx_->resultData.reset();
	lastGeneration_ = chartPlayer->getPlaybackGeneration();
	resultCaptured_ = false;
	endTransition_.Reset();
}

void PlayScene::Draw() {

	chartDrawer->Draw(*chartPlayer);
	if (resultCaptured_) {
		const int alpha = int(endTransition_.FadeOpacity() * 255);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void PlayScene::Update() {
	// Abort also works during the linger and never publishes a result.
	if (Input::isKeyTriggered(KEY_INPUT_BACK)) {
		ctx_->resultData.reset();
		chartPlayer->stopSong();
		RequestScene(SceneID::SongSelect);
		return;
	}

	// 自動判定と手動発光は、F1 切り替え後の同じモードを参照する。
	if (Input::isKeyTriggered(KEY_INPUT_F1)) {
		chartPlayer->toggleAutoplay();
	}
	chartPlayer->Update(ctx_->judgeOffsetUs);
	if (lastGeneration_ != chartPlayer->getPlaybackGeneration()) {
		lastGeneration_ = chartPlayer->getPlaybackGeneration();
		resultCaptured_ = false;
		ctx_->resultData.reset();
		endTransition_.Reset();
	}
	effects_->Update(*chartPlayer, Time::deltaSec());
	// リスタートによる演出初期化の後に、当該フレームの空打ちを追加する。
	Input();
	chartDrawer->Update(*chartPlayer);

	if (chartPlayer->isStoped) {
		ctx_->resultData.reset();
		chartPlayer->stopSong();
		RequestScene(SceneID::SongSelect);
		return;
	}
	if (chartPlayer->isFinished) {
		if (!resultCaptured_) {
			CaptureResult();
			resultCaptured_ = true;
			endTransition_.Reset();
		} else {
			endTransition_.Update(Time::deltaSec());
		}
		if (endTransition_.Complete()) {
			chartPlayer->stopSong();
			RequestScene(SceneID::Result);
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
void PlayScene::CaptureResult() {
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
		r.accuracy = float(ResultLogic::Calculate(ps.goodCount, ps.okCount, ps.badCount,
			ps.missCount, ps.rollHitCount, ps.balloonHitCount).Fraction());
	}
	
	ctx_->resultData = std::make_shared<ResultCtx>(r);
}

void PlayScene::Finalize() {
	chartPlayer->stopSong();
}


