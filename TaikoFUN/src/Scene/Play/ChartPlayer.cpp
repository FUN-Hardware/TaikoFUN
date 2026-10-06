#include "ChartPlayer.h"

#include "Input/Input.h"
#include "Audio/Sounds.h"

#include <cassert>

void ChartPlayer::Update() {
	nowSongTime = cd_.songData.getSongCurrentTimeUs( true );

	input();
	updateMissNotes();
	updateGogoTime();
	autoplayHitNote();
}

void ChartPlayer::input() {

	if ( !autoPlay ) {
		if ( Input::isNoteKeyTriggered( NoteType::Don ) ) {
			PlaySoundMem( Sounds::GetSoundHandle( SoundKey::Don ), DX_PLAYTYPE_BACK, true );
		}
		if ( Input::isNoteKeyTriggered( NoteType::Katsu ) ) {
			PlaySoundMem( Sounds::GetSoundHandle( SoundKey::Katsu ), DX_PLAYTYPE_BACK, true );
		}
	}

	if ( Input::isKeyTriggered( KEY_INPUT_SPACE ) ) {
		playSong( true );
	}

	if ( Input::isKeyTriggered( KEY_INPUT_F1 ) ) {
		autoPlay = !autoPlay;
	}

	if ( Input::isKeyTriggered( KEY_INPUT_KANJI ) ) playSong( true );


	judgeNote();

}

void ChartPlayer::init() {
	cd_.songData.stopSong();

	stats_.score = 0;
	stats_.combo = 0;
	stats_.maxCombo = 0;

	stats_.goodCount = 0;
	stats_.okCount = 0;
	stats_.badCount = 0;
	stats_.missCount = 0;
	nextNoteIndex = 0;

	for ( auto& Note : cd_.notes ) {
		Note.isJudged = false;
		Note.isMissed = false;
	}

	events_ = {};

}

void ChartPlayer::playSong( bool restart ) {
	if ( restart ) {
		init();
	}
	else {

	}


	cd_.songData.playSong( restart );
}

void ChartPlayer::nextNotes() {
	if ( nextNoteIndex < cd_.notes.size() - 1 ) {
		nextNoteIndex++;
		if ( cd_.notes[nextNoteIndex].type == NoteType::None || cd_.notes[nextNoteIndex].type == NoteType::RollTail ) {
			nextNotes();
		}
		lastRollIdx = cd_.notes[nextNoteIndex].idx;
	}

}

void ChartPlayer::updateMissNotes() {


	Note& targetNote = cd_.notes[nextNoteIndex];
	long long noteRelTime = noteRelativeTime( nextNoteIndex );

	if ( targetNote.isJudged || targetNote.isMissed ) return; // 既に判定済みまたはミス判定の出たノーツはスルー

	switch ( targetNote.type ) {
		case NoteType::Don: case NoteType::Katsu:
			if ( abs( noteRelTime ) > judgeBAD && noteRelTime < 0 ) {	// 判定ノーツを次に移行する条件は (ノーツの相対位置がマイナスであること) ⋏ (ノーツの絶対相対座標がjudgeBADの領域を超えていること) 要するに判定枠の後ろをBAD判定以上に進んでたら次ってこと
				targetNote.isMissed = true;
				events_.setJudgeEvent( nowSongTime, targetNote.type, targetNote.isBig, JudgeType::MISS );
				stats_.missCount++;
				stats_.combo = 0;
				nextNotes();
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

void ChartPlayer::judgeNote() {
	if ( !(cd_.notes.size() >= 1) ) return;
	Note& targetNote = cd_.notes[nextNoteIndex];

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

			stats_.maxCombo = max( stats_.combo, stats_.maxCombo );
			
			nextNotes();
			break;

		case NoteType::RollHead:
			assert( targetNote.pairRollIndex < cd_.notes.size() && "pairRollIndexが、範囲外です" );
			if ( targetNote.absTime < nowSongTime && nowSongTime < cd_.notes[targetNote.pairRollIndex].absTime ) {
				applyNoteJudge( targetNote, JudgeType::ROLLHIT );
			}
			break;

		case NoteType::BalloonHead:

			if ( targetNote.absTime < nowSongTime ) {
				applyNoteJudge( targetNote, JudgeType::BALLOONHIT );

				if ( targetNote.balloonHitCount >= targetNote.requiredHits ) {
					applyNoteJudge( targetNote, JudgeType::BALLOONCLEAR );
					PlaySoundMem( Sounds::GetSoundHandle( SoundKey::BalloonBreak ), DX_PLAYTYPE_BACK, true );
					nextNotes();
				}
			}
	}

}

long long ChartPlayer::noteRelativeTime( size_t noteIdx ) {
	return cd_.notes[noteIdx].absTime - nowSongTime;
}

void ChartPlayer::applyNoteJudge( Note& targetNote, JudgeType judgeType ) {

	events_.setJudgeEvent( nowSongTime, targetNote.type, targetNote.isBig, judgeType);

	float scoreMultiplier = 1.0f;
	if ( targetNote.isGogo ) scoreMultiplier *= GOGOSCOREMULTIPLIER;
	switch ( judgeType ) {
		case JudgeType::GOOD:
			stats_.score = scoreGOOD * scoreMultiplier;
			stats_.goodCount++;
			stats_.combo++;
			targetNote.isJudged = true;
			break;
		case JudgeType::OK:
			stats_.score += scoreOK * scoreMultiplier;
			stats_.okCount++;
			stats_.combo++;
			targetNote.isJudged = true;
			break;
		case JudgeType::BAD:
			targetNote.isJudged = true;
			stats_.badCount++;
			stats_.combo = 0;
			break;
		case JudgeType::MISS:
			targetNote.isMissed = true;
			stats_.combo = 0;
			stats_.missCount++;
			break;
		case JudgeType::ROLLHIT:
			stats_.score += scoreROLL;
			targetNote.rollHitCount++;
			break;
		case JudgeType::BALLOONHIT:
			stats_.score += scoreBALLOONHIT;
			targetNote.balloonHitCount++;
			break;
		case JudgeType::BALLOONCLEAR:
			stats_.score += scoreBALLOONCLEARED;
			targetNote.isJudged = true;
			break;
	}

}

void ChartPlayer::autoplayHitNote() {
	Note& targetNote = cd_.notes[nextNoteIndex];
	long long noteTime = noteRelativeTime( nextNoteIndex );
	if ( !(noteRelativeTime( nextNoteIndex ) <= 0) ) return;

	switch ( targetNote.type ) {
		case NoteType::RollHead:
			[[fallthrough]];
		case NoteType::BalloonHead:
			if ( lastRollHitUs > nowSongTime - rollIntervalUs ) return;
			if ( targetNote.type == NoteType::RollHead ) {
				if ( !(targetNote.absTime < nowSongTime && nowSongTime < cd_.notes[targetNote.pairRollIndex].absTime) ) return;
			}
			else {
				if ( !(targetNote.absTime < nowSongTime && nowSongTime < targetNote.absTime + targetNote.duration) ) return;
			}
			lastRollHitUs = nowSongTime;
			[[fallthrough]];
		case NoteType::Don:
			PlaySoundMem( Sounds::GetSoundHandle( SoundKey::Don ), DX_PLAYTYPE_BACK, true );
			break;
		case NoteType::Katsu:
			PlaySoundMem( Sounds::GetSoundHandle( SoundKey::Katsu), DX_PLAYTYPE_BACK, true);
			break;
	}

	switch ( targetNote.type ) {
		case NoteType::Don: case NoteType::Katsu:
			applyNoteJudge( targetNote, JudgeType::GOOD );
			if ( targetNote.isJudged ) nextNotes();
			break;

		case NoteType::RollHead:
			applyNoteJudge( targetNote, JudgeType::ROLLHIT );
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
