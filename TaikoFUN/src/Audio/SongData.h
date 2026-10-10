#pragma once

#include "SoundHandle.h"

struct SongData
{

	long long songStartTime;	// 曲の絶対開始時間 (us)
	long long _songProgTime;		// 曲の経過時間(再生位置) (us)
	long long offsetTime;		// 曲のオフセット時間(譜面の開始位置) (us)
	long long _songProgTimefromOffset;	// 曲のオフセットからの経過時間 (us)
	long long lastPausedTimeStamp;	// 最後に一時停止した時間 (us)
	long long totalPausedDuration; // 一時停止した時間の合計 (us)

	SoundHandle songHandle;				// 曲のハンドル
	bool playing = false;


	void playSong( bool restart = false );
	void playSongFrom( long long startAt );
	void stopSong();
	void loadSong( const char* path );
	long long getSongCurrentTimeUs( bool applyOffset = false );

};
