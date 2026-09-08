#include "ChartData.h"
#include "dxlib.h"
#include "Time.h"
#include "Input/Input.h"
#include "Skin/SkinData.h"

#include <algorithm>

std::string GetNoteImageKey(NoteType type) {
	switch (type) {
	case NoteType::Don: return "note/Don";
	case NoteType::Katsu: return "note/Katsu";
	case NoteType::DonBig: return "note/BigDon";
	case NoteType::KatsuBig: return "note/BigKatsu";
	case NoteType::Judge: return "note/Judgeframe";
	}
	return "";
}

void SongData::playSong(bool restart) {
	songHandle.play(restart);
	playing = true;
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
	playing = false;
}

void SongData::loadSong(const char* path) {
	songHandle.load(path);
}

long long SongData::getSongCurrentTimeUs(bool applyOffset) {
	_songProgTime = GetSoundCurrentTime(songHandle.handle) * 1000.0;
	if (applyOffset) _songProgTime -= offsetTime;
	return _songProgTime;

}



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


long long ChartData::noteRelativeTime(size_t noteIdx) {
	return notes[noteIdx].absTime - songData.getSongCurrentTimeUs(true);
}


void ChartData::init() {
	songData.stopSong();

	score = 0;
	combo = 0;
	MAXcombo = 0;

	good = 0;
	ok = 0;
	bad = 0;
	miss = 0;
	nextNoteIndex = 0;

	for (auto& Note : notes) {
		Note.isJudged = false;
		Note.isMissed = false;
	}

	judgelogs.clear();

}


void ChartData::Update() {

	updateMissNotes();

}

void ChartData::Input() {


	if (Input::isNoteKeyTriggered(NoteType::Don)) {
		PlaySoundMem(Skin::GetSound("Don").handle, DX_PLAYTYPE_BACK, true);
	}
	if (Input::isNoteKeyTriggered(NoteType::Katsu)) {
		PlaySoundMem(Skin::GetSound("Katsu").handle, DX_PLAYTYPE_BACK, true);
	}

	if (Input::isKeyTriggered(KEY_INPUT_SPACE)) {
		playSong(true);
	}

	if (Input::isKeyTriggered(KEY_INPUT_KANJI)) playSong(true);


	judgeNote();

}

void ChartData::updateMissNotes() {

	
	Note& targetNote = notes[nextNoteIndex];
	long long noteRelTime = noteRelativeTime(nextNoteIndex);

	if (targetNote.isJudged || targetNote.isMissed) return; // 既に判定済みまたはミス判定の出たノーツはスルー

	if (abs(noteRelTime) > judgeBAD && noteRelTime < 0) {	// 判定ノーツを次に移行する条件は (ノーツの相対位置がマイナスであること) ⋏ (ノーツの絶対相対座標がjudgeBADの領域を超えていること) 要するに判定枠の後ろをBAD判定以上に進んでたら次ってこと
		targetNote.isMissed = true;
		judgelogs.push_back({JudgeType::MISS, songData.getSongCurrentTimeUs()});
		miss++;
		combo = 0;
		nextNotes();
		//PlaySoundMem(Skin::GetSound("Katsu").handle, DX_PLAYTYPE_BACK, true);	//デバッグ用
	}
	

}

void ChartData::nextNotes() {
	if (nextNoteIndex < notes.size()-1) {
		nextNoteIndex++;
		if (notes[nextNoteIndex].type == NoteType::None) {
			nextNotes();
		}
	}
}


void ChartData::judgeNote() {

	if (!(notes.size() >= 1)) return;
	Note& targetNote = notes[nextNoteIndex];

	if (abs(noteRelativeTime(nextNoteIndex)) > judgeBAD) return; // 判定対象の相対位置がjudgeBad判定領域より大きければリターン


	if (!Input::isNoteKeyTriggered(targetNote.type)) return; // 判定対象のノーツタイプの入力が無ければリターン


	targetNote.isJudged = true;
	if (abs(noteRelativeTime(nextNoteIndex)) < judgeGOOD) {	// 良判定
		judgelogs.push_back({ JudgeType::GOOD, songData.getSongCurrentTimeUs() });
		score += scoreGOOD;
		good++;
		combo++;
	}
	else if (abs(noteRelativeTime(nextNoteIndex)) < judgeOK) {	// 可判定
		judgelogs.push_back({ JudgeType::OK, songData.getSongCurrentTimeUs() });
		score += scoreOK;
		ok++;
		combo++;
	}
	else if (abs(noteRelativeTime(nextNoteIndex)) < judgeBAD) {	// 不可判定
		judgelogs.push_back({ JudgeType::BAD, songData.getSongCurrentTimeUs() });
		score += scoreBAD;
		bad++;
		combo = 0;
	}

	MAXcombo = max(combo, MAXcombo);
	nextNotes();
}
