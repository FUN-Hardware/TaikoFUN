#include "ChartData.h"
#include "dxlib.h"


void SongData::playSong() {
	PlaySoundMem(songHandle, DX_PLAYTYPE_BACK);
	songStartTime = GetNowHiPerformanceCount();
	_songProgTime = 0;
}

void SongData::loadSong(const char* path) {
	songHandle = LoadSoundMem(path);
}



void ChartData::loadSong(const char* path, double bpm) {
	songData.loadSong(path);
	this->bpm = bpm;
}

