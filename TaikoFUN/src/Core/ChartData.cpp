#include "ChartData.h"
#include "dxlib.h"
#include "Time.h"

void SongData::playSong(bool restart) {
	songHandle.play(restart);
	if (restart) {
		totalPausedDuration = 0;
		lastPausedTimeStamp = 0;
		songStartTime = GetNowHiPerformanceCount();
		_songProgTime = 0;
	}
	else {
		if(lastPausedTimeStamp != 0)totalPausedDuration += (Time::nowTime() - lastPausedTimeStamp);
	}
}

void SongData::stopSong() {
	lastPausedTimeStamp = Time::nowTime();
	songHandle.stop();
}

void SongData::loadSong(const char* path) {
	songHandle.load(path);
}



void ChartData::loadSong(const char* path, double bpm, double offset) {
	songData.loadSong(path);
	this->bpm = bpm;
	this->songData.offsetTime = offset * 1000000;
}

void ChartData::playSong(bool restart) {
	if (restart) {
		for (auto& Note : notes) Note.isJudged = false;
		
		nextNoteIndex = 0;
	}
	else {
		
	}


	songData.playSong(restart);
}

