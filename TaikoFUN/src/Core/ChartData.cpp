#include "ChartData.h"
#include "dxlib.h"
#include "Time.h"
#include "Input/Input.h"


std::string GetNoteImageKey(NoteType type) {
	switch (type) {
	case NoteType::Don: return "note/Don";
	case NoteType::Katsu: return "note/Katsu";
	case NoteType::DonBig: return "note/BidDon";
	case NoteType::KatsuBig: return "note/BigKatsu";
	case NoteType::Judge: return "note/Judgeframe";
	}
	return "";
}

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

long long SongData::getSongCurrentTimeUs(bool applyOffset) {
	_songProgTime = GetSoundCurrentTime(songHandle.handle) * 1000.0;
	if (applyOffset) _songProgTime -= offsetTime;
	return _songProgTime;

}




void ChartData::loadSong(const char* path, double bpm, double offset) {
	songData.loadSong(path);
	this->bpm = bpm;
	this->songData.offsetTime = offset * 1000000.0;
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


long long ChartData::noteRelativeTime(size_t noteIdx) {
	return notes[noteIdx].absTime - songData.getSongCurrentTimeUs(true);
}

void ChartData::Update() {

	updateMissNotes();

}

void ChartData::updateMissNotes() {

	
	Note& targetNote = notes[nextNoteIndex];
	long long noteRelTime = noteRelativeTime(nextNoteIndex);

	if (targetNote.isJudged || targetNote.isMissed) return; // 既に判定済みまたはミス判定の出たノーツはスルー

	if (abs(noteRelTime) < judgeBAD && noteRelTime < 0) {	// 判定ノーツを次に移行する条件は (ノーツの相対位置がマイナスであること) ⋏ (ノーツの絶対相対座標がjudgeBADの領域を超えていること) 要するに判定枠の後ろをBAD判定以上に進んでたら次ってこと
		targetNote.isMissed = true;
		judgelogs.push_back({JudgeType::MISS, songData.getSongCurrentTimeUs()});
		miss++;
		this->nextNoteIndex++;
	}
	

}

void ChartData::Input() {

	judgeNote();
}

void ChartData::judgeNote() {

	Note& targetNote = notes[nextNoteIndex];

	if (abs(noteRelativeTime(nextNoteIndex)) > judgeBAD) return; // 判定対象の相対位置がjudgeBad判定領域より大きければリターン


	if (!Input::isNoteKeyTriggered(targetNote.type)) return; // 判定対象のノーツタイプの入力が無ければリターン


	targetNote.isJudged = true;
	if (abs(noteRelativeTime(nextNoteIndex)) < judgeGOOD) {	// 良判定
		judgelogs.push_back({ JudgeType::GOOD, songData.getSongCurrentTimeUs() });
		score += scoreGOOD;
		good++;
	}
	else if (abs(noteRelativeTime(nextNoteIndex)) < judgeOK) {	// 可判定
		judgelogs.push_back({ JudgeType::OK, songData.getSongCurrentTimeUs() });
		score += scoreOK;
		ok++;
	}
	else if (abs(noteRelativeTime(nextNoteIndex)) < judgeBAD) {	// 不可判定
		judgelogs.push_back({ JudgeType::BAD, songData.getSongCurrentTimeUs() });
		score += scoreBAD;
		bad++;
	}

	nextNoteIndex++;
}
