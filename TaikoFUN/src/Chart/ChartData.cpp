#include "ChartData.h"
#include "dxlib.h"
#include "Core/Time.h"
#include "Input/Input.h"
#include "Skin/SkinData.h"

#include <algorithm>
#include <cassert>

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

long long SongData::getSongCurrentTimeUs(bool applyOffset) const{
	long long songProgTime = GetSoundCurrentTime(songHandle.handle) * 1000.0;
	if (applyOffset) songProgTime -= offsetTime;
	return songProgTime;

}


void ChartData::Update() {

	nowTime = Time::nowTime();
	nowSongTime = songData.getSongCurrentTimeUs( true );
	updateMissNotes();
	updateGogoTime();

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
	return notes[noteIdx].absTime - nowSongTime;
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



void ChartData::Input() {

	if (!autoPlay) {
		if (Input::isNoteKeyTriggered(NoteType::Don)) {
			PlaySoundMem(Skin::GetSound("Don").handle, DX_PLAYTYPE_BACK, true);
		}
		if (Input::isNoteKeyTriggered(NoteType::Katsu)) {
			PlaySoundMem(Skin::GetSound("Katsu").handle, DX_PLAYTYPE_BACK, true);
		}
	}

	if (Input::isKeyTriggered(KEY_INPUT_SPACE)) {
		playSong(true);
	}

	if (Input::isKeyTriggered(KEY_INPUT_F1)) {
		autoPlay = !autoPlay;
	}

	if (Input::isKeyTriggered(KEY_INPUT_KANJI)) playSong(true);


	judgeNote();

}


void ChartData::updateMissNotes() {

	
	Note& targetNote = notes[nextNoteIndex];
	long long noteRelTime = noteRelativeTime(nextNoteIndex);

	if (targetNote.isJudged || targetNote.isMissed) return; // 既に判定済みまたはミス判定の出たノーツはスルー

	switch ( targetNote.type ) {
		case NoteType::Don: case NoteType::Katsu:
			if ( abs( noteRelTime ) > judgeBAD && noteRelTime < 0 ) {	// 判定ノーツを次に移行する条件は (ノーツの相対位置がマイナスであること) ⋏ (ノーツの絶対相対座標がjudgeBADの領域を超えていること) 要するに判定枠の後ろをBAD判定以上に進んでたら次ってこと
				targetNote.isMissed = true;
				judgelogs.push_back( { JudgeType::MISS, nowSongTime } );
				miss++;
				combo = 0;
				nextNotes();
				//PlaySoundMem(Skin::GetSound("Katsu").handle, DX_PLAYTYPE_BACK, true);	//デバッグ用
			}
			break;

		case NoteType::RollHead:
			if ( !(noteRelativeTime( targetNote.pairRollIndex ) <= 0) )	return; // 連打ノーツの尾がすぎるまで進行しない
			targetNote.isJudged = true;
			nextNotes();
			break;
	
		case NoteType::BalloonHead:
			if ( !(noteRelativeTime( targetNote.idx ) + targetNote.duration <= 0) )	return; // 風船ノーツの尾がすぎるまで進行しない
			targetNote.isMissed = true;
			nextNotes();
			break;
	}

}

void ChartData::nextNotes() {
	if (nextNoteIndex < notes.size()-1) {
		nextNoteIndex++;
		if (notes[nextNoteIndex].type == NoteType::None || notes[nextNoteIndex].type == NoteType::RollTail ) {
			nextNotes();
		}
		lastRollIdx = notes[nextNoteIndex].idx;
	}
}


void ChartData::judgeNote() {

	if ( !(notes.size() >= 1) ) return;
	Note& targetNote = notes[nextNoteIndex];

	if ( autoPlay ) {
		autoplayHitNote();
		return;
	}

	if ( abs( noteRelativeTime( nextNoteIndex ) ) > judgeBAD ) return; // 判定対象の相対位置がjudgeBad判定領域より大きければリターン





	switch ( targetNote.type ) {	// switch文で入力を判定し、想定される入力ではない場合、早期リターン
		case NoteType::Don:
			if ( !Input::isNoteKeyTriggered( NoteType::Don ) ) return;
			break;
		case NoteType::Katsu:
			if ( !Input::isNoteKeyTriggered( NoteType::Katsu ) ) return;
			break;
		case NoteType::BalloonHead:
			if ( !Input::isNoteKeyTriggered( NoteType::Don ) ) return;
	}

	//if (!Input::isNoteKeyTriggered(targetNote.type)) return; // 判定対象のノーツタイプの入力が無ければリターン

	switch ( targetNote.type ) {
		case NoteType::Don: case NoteType::Katsu:
			targetNote.isJudged = true;
			if ( abs( noteRelativeTime( nextNoteIndex ) ) < judgeGOOD ) {	// 良判定
				applyNoteJudge( targetNote, JudgeType::GOOD );
			}
			else if ( abs( noteRelativeTime( nextNoteIndex ) ) < judgeOK ) {	// 可判定
				applyNoteJudge( targetNote, JudgeType::OK );
			}
			else if ( abs( noteRelativeTime( nextNoteIndex ) ) < judgeBAD ) {	// 不可判定
				applyNoteJudge( targetNote, JudgeType::BAD );
			}

			MAXcombo = max( combo, MAXcombo );
			nextNotes();
			break;

		case NoteType::RollHead:
			assert( targetNote.pairRollIndex < notes.size() && "pairRollIndexが、範囲外です" );
			if ( targetNote.absTime < nowSongTime && nowSongTime < notes[targetNote.pairRollIndex].absTime ) {
				applyNoteJudge( targetNote, JudgeType::ROLLHIT );
			}
			break;

		case NoteType::BalloonHead:

			if ( targetNote.absTime < nowSongTime ) {
				applyNoteJudge( targetNote, JudgeType::BALLOONHIT );

				if ( targetNote.balloonHitCount >= targetNote.requiredHits ) {
					applyNoteJudge( targetNote, JudgeType::BALLOONCLEAR );
					nextNotes();
				}
			}
	}
}

void ChartData::autoplayHitNote() {
	Note& targetNote = notes[nextNoteIndex];
	long long noteTime = noteRelativeTime( nextNoteIndex );
	if ( !(noteRelativeTime( nextNoteIndex ) <= 0) ) return;

	switch ( targetNote.type ) {
		case NoteType::RollHead:
			[[fallthrough]];
		case NoteType::BalloonHead:
			if ( lastRollHitUs > nowSongTime - rollIntervalUs) return;
			if ( targetNote.type == NoteType::RollHead ) {
				if ( !(targetNote.absTime < nowSongTime && nowSongTime < notes[targetNote.pairRollIndex].absTime) ) return;
			}
			else {
				if ( !(targetNote.absTime < nowSongTime && nowSongTime < targetNote.absTime + targetNote.duration) ) return;
			}
			lastRollHitUs = nowSongTime;
			[[fallthrough]];
		case NoteType::Don:
			PlaySoundMem( Skin::GetSound( "Don" ).handle, DX_PLAYTYPE_BACK, true );
			break;
		case NoteType::Katsu:
			PlaySoundMem( Skin::GetSound( "Katsu" ).handle, DX_PLAYTYPE_BACK, true );
			break;
	}

	switch ( targetNote.type ) {
		case NoteType::Don: case NoteType::Katsu:
			applyNoteJudge( targetNote, JudgeType::GOOD );
			if ( targetNote.isJudged ) nextNotes();
			break;

		case NoteType::RollHead:
			applyNoteJudge( targetNote, JudgeType::ROLLHIT);
			break;

		case NoteType::BalloonHead:
			applyNoteJudge( targetNote, JudgeType::BALLOONHIT );
			if ( targetNote.balloonHitCount >= targetNote.requiredHits ) {
				applyNoteJudge( targetNote, JudgeType::BALLOONCLEAR );
				nextNotes();
			}
			break;
	}
}

void ChartData::applyNoteJudge(Note& targetNote, JudgeType judgeType) {
	
	judgelogs.push_back( { judgeType, nowSongTime } );
	float scoreMultiplier = 1.0f;
	if ( targetNote.isGogo ) scoreMultiplier *= GOGOSCOREMULTIPLIER;
	switch ( judgeType ) {
		case JudgeType::GOOD:
			score = scoreGOOD * scoreMultiplier;
			good++;
			combo++;
			targetNote.isJudged = true;
			break;
		case JudgeType::OK:
			score += scoreOK * scoreMultiplier;
			ok++;
			combo++;
			targetNote.isJudged = true;
			break;
		case JudgeType::BAD:
			targetNote.isJudged = true;
			bad++;
			combo = 0;
			break;
		case JudgeType::MISS:
			targetNote.isMissed = true;
			combo = 0;
			miss++;
			break;
		case JudgeType::ROLLHIT:
			score += scoreROLL;
			targetNote.rollHitCount++;
			break;
		case JudgeType::BALLOONHIT:
			score += scoreBALLOONHIT;
			targetNote.balloonHitCount++;
			break;
		case JudgeType::BALLOONCLEAR:
			score += scoreBALLOONCLEARED;
			targetNote.isJudged = true;
			break;
	}

}


void ChartData::updateGogoTime() {
	if (gogoTimes.size() == 0) return;
	if (gogoIndex >= gogoTimes.size()) return;

	if ( nowSongTime >= gogoTimes[gogoIndex].startTime && nowSongTime < gogoTimes[gogoIndex].endTime ) { 
		isGogoTime = true;
	}
	else {
		isGogoTime = false;
	}
}