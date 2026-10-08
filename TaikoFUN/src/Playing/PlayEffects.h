#pragma once

#include "Core/ChartData.h"

#include <vector>

// 演奏画面の演出（判定文字・コンボ数・ヒット効果・レーンの光）。
// ChartData は読み取るだけで、ゲームの進行や判定には一切影響しない。
class PlayEffects
{
public:
	// 毎フレーム、CD.Input()/CD.Update() の後に呼ぶ
	void Update(const ChartData& cd, double dtSec);

	// レーンの上・ノーツより下に描く（レーンの光）
	void DrawLane() const;
	// ノーツより上に描く（ヒット効果・判定文字・コンボ）
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

	size_t processedJudgeLogs = 0;
	std::vector<HitBurst> bursts;
	std::vector<LaneFlash> laneFlashes;
	JudgePop judgePop{ JudgeType::GOOD, 1e9 };

	int combo = 0;
	double comboAge = 1e9;		// 直近でコンボが増えてからの秒数
	double milestoneAge = 1e9;	// 100コンボごとの大きな演出からの秒数
	int brokenCombo = 0;		// 途切れたコンボ（落下させて消す）
	double brokenAge = 1e9;

	void Reset();
	void OnJudge(const ChartData& cd, const JudgeLog& log);
};
