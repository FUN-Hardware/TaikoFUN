#pragma once

#include "Scene/Play/Logic/ChartPlayer.h"

#include <array>
#include <vector>

// 演奏画面の演出（判定文字・コンボ数・ヒット効果・レーンの光）。
// ChartPlayer の判定イベントと統計は読み取るだけで、ゲームの進行や判定には影響しない。
class PlayEffects
{
public:
	// ミニ太鼓を正面から見た左右。D / F / J / K の順。
	enum class DrumPart { LeftRim, LeftFace, RightFace, RightRim };
	// 手動入力は判定成否に関係なく、押した部位だけ光らせる。
	void TriggerMiniDrumFlash(DrumPart part);

	// 手動打鍵（空打ち含む）によるレーン発光トリガー
	void TriggerLaneFlash(NoteType noteType);

	// 毎フレーム、ChartPlayer::Update() の後に呼ぶ
	void Update(const ChartPlayer& player, double dtSec);

	// レーンの上・ノーツより下に描く（レーンの光）
	void DrawLane() const;
	// レーンの光より上・ノーツより下に描く（ヒット効果）
	void DrawHitEffects() const;
	// レーン左の空きに既存ポスター素材を組み合わせたミニ太鼓を描く
	void DrawMiniDrum() const;
	// ノーツより上に描く（判定文字・コンボ）
	void DrawOverlay() const;

private:
	enum class HitColor { Don, Katsu, Roll };

	struct HitBurst {
		HitColor color;
		bool big;
		bool good;
		double age = 0.0;
		double spin;
	};
	struct LaneFlash {
		HitColor color;
		double age = 0.0;
	};
	struct JudgePop {
		JudgeType type;
		double age = 0.0;
	};

	size_t processedJudgeEvents = 0;
	size_t playbackGeneration = 0;
	std::vector<HitBurst> bursts;
	std::vector<LaneFlash> laneFlashes;
	JudgePop judgePop{ JudgeType::GOOD, 1e9 };
	std::array<double, 4> drumFlashAges{ 1e9, 1e9, 1e9, 1e9 };
	bool autoDrumRight = false;

	int combo = 0;
	double comboAge = 1e9;		// 直近でコンボが増えてからの秒数
	double milestoneAge = 1e9;	// 100コンボごとの大きな演出からの秒数
	int brokenCombo = 0;		// 途切れたコンボ（落下させて消す）
	double brokenAge = 1e9;

	void Reset();
	void TriggerAutoDrumFlash(NoteType noteType, bool big);
	void OnJudge(const ChartPlayer& player, const JudgeEvent& event);
};
