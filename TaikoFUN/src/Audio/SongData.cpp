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

