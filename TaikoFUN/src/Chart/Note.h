#pragma once

#include <climits>

enum class NoteType
{
	None,
	Don,
	Katsu,
	RollHead,
	RollTail,
	BalloonHead,
	Judge,
};

struct Note
{

	double bpm = 120.0;				// ノーツの速度(流れる速さはこれに依存する)			
	long long absTime = 0;		// ノーツの絶対座標　(曲オフセットからの相対時間)	ノーツが流れてくる位置
	NoteType type = NoteType::None;				// ノーツタイプ (1=ドン, 2=カツ, 3=大ドン, 4=大カツ)
	double scroll = 1.0;		// ノーツのスクロール速度 (譜面読み込み時に計算して結果をここに入れる)	描画時にbpmにこれを掛けて描画
	bool hasBarline = false;	// 小節線の有無
	bool isBig = false;			// 大音符かどうか
	bool isGogo = false; 		// ゴーゴータイム中のノーツかどうか

	size_t idx = SIZE_MAX;	// notesにおける自身のインデックス
	bool isJudged = false;  // 既に判定を下かどうかを保持
	bool isMissed = false;	// ノーツを見逃したかどうかのフラグ

	// 連打用パラメータ
	size_t pairRollIndex = SIZE_MAX;	// 連打尾のインデックスを保持
	size_t rollId = SIZE_MAX;						// パース時に使用する連打のペアを保証するID

	size_t rollHitCount = 0;			// 譜面再生時に連打した回数を保持

	// 風船用パラメータ	風船は連打と違い、尾を持たずに頭に情報を持たせる。
	size_t balloonId = SIZE_MAX;			// パース時に使用する風船のペアを保証するID
	size_t requiredHits = 0;	// 風船を割るのに必要なヒット数
	size_t balloonHitCount = 0;	// 譜面再生時に風船を叩いた回数を保持
	long long duration = 0;	// 風船の持続時間(譜面上の長さ)を保持するためのパラメータ
};
