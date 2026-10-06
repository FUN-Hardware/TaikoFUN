#pragma once


// ゲームプレイ中の統計情報を保持する構造体
// リザルトにも渡す
struct PlayStats
{
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

	void Reset() {
		score = 0;
		combo = 0;
		maxCombo = 0;
		goodCount = 0;
		okCount = 0;
		badCount = 0;
		missCount = 0;
		rollHitCount = 0;
		balloonHitCount = 0;
		balloonClearCount = 0;
	}
};