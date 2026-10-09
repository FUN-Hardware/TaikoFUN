#pragma once

#include "Chart/Note.h"
enum class JudgeType
{
	GOOD,
	OK,
	BAD,
	MISS,
	ROLLHIT,
	BALLOONHIT,
	BALLOONCLEAR,
};

struct JudgeEvent	// ChartPlayerが判定時に生成され、ChartDrawerが保持し描画に使用
{
	long long timestamp = 0; // 判定イベントのタイムスタンプ (us)
	NoteType noteType = NoteType::None;   // 判定されたノーツの種類
	JudgeType judgeType; // 判定結果の種類
	bool isBig = false;
	void setJudgeEvent( long long ts, NoteType nt, bool ib, JudgeType jt )
	{ 
		timestamp = ts;
		noteType = nt;
		isBig = ib;
		judgeType = jt;
	}
};