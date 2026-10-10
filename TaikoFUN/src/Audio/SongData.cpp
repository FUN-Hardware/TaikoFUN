#include "SongData.h"

#include "Core/Time.h"
void SongData::playSong( bool restart ) {
	songHandle.play( restart );
	playing = true;
	if ( restart ) {
		totalPausedDuration = 0;
		lastPausedTimeStamp = 0;
		songStartTime = Time::nowTime();
		_songProgTime = 0;
	}
	else {
		if ( lastPausedTimeStamp != 0 )totalPausedDuration += (Time::nowTime() - lastPausedTimeStamp);
	}
}

void SongData::playSongFrom( long long startAt ) {

	songHandle.play( true );
	SetCurrentPositionSoundMem( startAt, songHandle.handle );

}

// SongData に追加するイメージ
long long SongData::getSmoothTimeUs( bool applyOffset ) {
	long long nowCounter = GetNowHiPerformanceCount();      // マイクロ秒単位のタイマー
	long long audioUs = GetSoundCurrentTime( songHandle.handle ) * 1000LL;

	if ( audioUs != lastAudioUs_ ) {                         // 音源の値が進んだ瞬間だけ基準を更新する
		lastAudioUs_ = audioUs;
		lastCounterUs_ = nowCounter;
	}
	long long t = lastAudioUs_ + (nowCounter - lastCounterUs_);   // 間の時間はタイマーで補う
	if ( applyOffset ) t -= offsetTime;
	return t;
}

void SongData::stopSong() {
	lastPausedTimeStamp = Time::nowTime();
	songHandle.stop();
	playing = false;
}

void SongData::loadSong( const char* path ) {
	songHandle.load( path );
}

long long SongData::getSongCurrentTimeUs( bool applyOffset ) {
	_songProgTime = GetSoundCurrentTime( songHandle.handle ) * 1000.0;
	if ( applyOffset ) _songProgTime -= offsetTime;
	return _songProgTime;

}

