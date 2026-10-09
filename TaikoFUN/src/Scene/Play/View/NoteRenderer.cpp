#include "NoteRenderer.h"

#include "Core/General.h"
#include "Skin/SkinData.h"
#include "Chart/Note.h"
#include "SkinLayout.h"
#include <unordered_set>



void NoteRenderer::Draw() {



	int noteX;
	long long noteRelativeTime; // 曲の再生位置によるノーツの相対時間(us)
	/// ノーツ描画
	////////////////　ヘルパー関数
	auto isDrawable = []( const int x, const bool isJudged ) {
		int winx, winy;
		GetWindowSize( &winx, &winy );

		bool isInsideScreen = (x < winx) &&
			(x > -SkinLayout::NoteSize);
		return isInsideScreen && !isJudged;
	};

	auto isRangeDrawable = []( const int x1, const int x2, const bool isJudged ) {
		int winx, winy;
		GetWindowSize( &winx, &winy );

		bool isInsideScreen = (x1 < winx) && (x2 > 0);
		return isInsideScreen && !isJudged;
	};

	auto getNoteXFromRelativeTime = [this]( double relativeTimeSec, double bpm, double scroll ) { // 汎用
		return (relativeTimeSec / (240.0 / bpm)) * 900.0 * scroll + SkinLayout::ScrollField.x;
	};

	auto getNoteX = [this]( const Note& note ) {
		return ((cd_.noteRelativeTime( note.idx ) / 1000000.0) / (240.0 / note.bpm)) * 900.0 * note.scroll + SkinLayout::ScrollField.x;
	};

	auto DrawNote = [&getNoteX]( const Note& note, const long long& relativeTimeUs, std::string& textureKey ) {
		double noteX = getNoteX( note );
		DrawExtendGraph( noteX,
						 SkinLayout::ScrollField.y,
						 noteX + SkinLayout::NoteSize,
						 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
						 Skin::GetTexture( textureKey ).handle,
						 true );
	};

	auto DrawBalloon = [this, &getNoteXFromRelativeTime]( const Note& note, std::string& HeadImgKey, std::string& TailImgKey ) {
		/// 風船ノーツの描画関数。
		//  
		// 風船ノーツは連打中に判定枠にとどまるため、連打開始前と連打終了後で座標計算に使う時間を変える必要がある


		double noteX = 0;
		long long rel = cd_.noteRelativeTime( note.idx );
		long long dur = note.duration;
		if ( rel > 0 ) {	// 連打開始前
			noteX = getNoteXFromRelativeTime( rel / 1000000.0, note.bpm, note.scroll ); // 相対時間をそのまま渡す
		}
		else if ( rel + note.duration < 0 ) {	// 連打終了後
			noteX = getNoteXFromRelativeTime( (rel + dur) / 1000000.0, note.bpm, note.scroll ); // 相対時間をそのまま渡す
		}
		else { // 連打中
			noteX = getNoteXFromRelativeTime( 0.0, note.bpm, note.scroll ); // 相対時間を0で渡して、判定枠にとどめる
		}

		if ( note.isJudged ) return; // 判定済みなら描画しない
		DrawExtendGraph( noteX,
						 SkinLayout::ScrollField.y,
						 noteX + SkinLayout::NoteSize,
						 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
						 Skin::GetTexture( HeadImgKey ).handle,
						 true );
		noteX += SkinLayout::NoteSize;
		DrawExtendGraph( noteX,
						 SkinLayout::ScrollField.y,
						 noteX + SkinLayout::NoteSize,
						 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
						 Skin::GetTexture( TailImgKey ).handle,
						 true );
	};


	//////////////////
	size_t index = 0;
	const size_t notesIndex = cd_.nextNoteIndex;

	std::unordered_set<size_t> drawnRolls;
	for ( size_t i = cd_.notes.size(); i-- > 0; ) {
		const auto& note = cd_.notes[i];
		noteRelativeTime = cd_.noteRelativeTime( cd_.notes.size() - 1 - index );
		noteX = getNoteX( note );
		// 240/BPM = 1小節の秒数。1小節当たり960pxとする。よって、(相対時間)/(240/BPM) * 960 = ノーツのX座標
		NoteType drawType = note.type;
		if ( note.type == NoteType::None ) {
			index++;
			continue; // 0(空白ノーツは描画しない)
		}
		std::string targetNoteTextureKey;

		switch ( drawType ) {
			case NoteType::Don: case NoteType::Katsu:
				if ( !isDrawable( noteX, note.isJudged ) )  break;
				targetNoteTextureKey = Skin::GetNoteImageKey( drawType, note.isBig );
				DrawNote( note, noteRelativeTime, targetNoteTextureKey );
				break;
			//

			case NoteType::RollHead:
			case NoteType::RollTail:
			{											// 連打ノーツの描画。 連打頭と連打尾の両方を通すが、二重描画を防ぐため描画済みかどうかを検知する。

				if ( drawnRolls.contains( note.rollId ) ) break;	// 既に描画済みの連打はスルー

				bool isRollHead = (note.type == NoteType::RollHead);

				const Note& rollHead = isRollHead ? note : cd_.notes[note.pairRollIndex];
				const Note& rollTail = !isRollHead ? note : cd_.notes[note.pairRollIndex];


				size_t rollHeadIdx = rollTail.pairRollIndex;
				size_t rollTailIdx = rollHead.pairRollIndex;

				double rollHeadX = getNoteX( rollHead );
				double rollTailX = getNoteX( rollTail );

				bool drawRoll = (isRangeDrawable( rollHeadX, rollTailX + SkinLayout::NoteSize, false ));
				bool isBig = rollHead.isBig;

				if ( !drawRoll ) break;


				std::string headImgKey = Skin::GetRollImageKey( Skin::RollPart::Head, isBig );
				std::string tailImgKey = Skin::GetRollImageKey( Skin::RollPart::Tail, isBig );
				std::string bodyImgKey = Skin::GetRollImageKey( Skin::RollPart::Body, isBig );


				DrawExtendGraph( rollTailX,
				 SkinLayout::ScrollField.y,
				 rollTailX + SkinLayout::NoteSize,
				 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
				 Skin::GetTexture( tailImgKey ).handle,
				 true );

				DrawExtendGraph( rollHeadX + SkinLayout::NoteSize / 2,
								 SkinLayout::ScrollField.y,
								 rollTailX + SkinLayout::NoteSize / 2,
								 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
								 Skin::GetTexture( bodyImgKey ).handle,
								 true );


				DrawExtendGraph( rollHeadX,
								 SkinLayout::ScrollField.y,
								 rollHeadX + SkinLayout::NoteSize,
								 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
								 Skin::GetTexture( headImgKey ).handle,
								 true );



				break;

			}

			case NoteType::BalloonHead:
			{
				std::string balloonHeadImgKey = Skin::GetBalloonImageKey( Skin::RollPart::Head );
				std::string balloonTailImgKey = Skin::GetBalloonImageKey( Skin::RollPart::Tail );

				DrawBalloon( note, balloonHeadImgKey, balloonTailImgKey );
				break;
			}


		}

		index++;
	}

}