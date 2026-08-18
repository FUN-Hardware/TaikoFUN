#include "ChartData.h"
#include "dxlib.h"


void SongData::playSong(bool restart) {
	songHandle.play(restart);
	songStartTime = GetNowHiPerformanceCount();
	_songProgTime = 0;
}

void SongData::loadSong(const char* path) {
	songHandle.load(path);
}



void ChartData::loadSong(const char* path, double bpm, double offset) {
	songData.loadSong(path);
	this->bpm = bpm;
	this->songData.offsetTime = offset * 1000000;
}

