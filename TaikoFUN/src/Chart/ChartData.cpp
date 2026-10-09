#include "ChartData.h"
#include "dxlib.h"
#include "Core/Time.h"
#include "Input/Input.h"
#include "Skin/SkinData.h"

#include <algorithm>
#include <cassert>



void ChartData::loadSong(const char* path) {
	songData.loadSong(path);
	this->songData.offsetTime = offset * 1000000.0;
}

void ChartData::loadSong(const char* path, double bpm, double _offset) {
	
	songData.loadSong(path);
	this->bpm = bpm;
	this->songData.offsetTime = _offset * 1000000.0;
}

void ChartData::playSong(bool restart) {
	if (restart) {
		init();
	}
	else {
		
	}


	songData.playSong(restart);
}

void ChartData::init() { 

	for ( auto n : notes ) {

		n.isJudged = false;
		n.isMissed = false;
		n.rollHitCount = 0;
		n.balloonHitCount = 0;
	}

}; // 初期化処理

long long ChartData::noteRelativeTime(size_t noteIdx) {
	return notes[noteIdx].absTime - songData.getSongCurrentTimeUs(true);
}



