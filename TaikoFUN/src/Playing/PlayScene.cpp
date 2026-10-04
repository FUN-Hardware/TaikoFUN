#include "DxLib.h"
#include "PlayScene.h"
#include "Debug/FPS.h"

#include "Input/Input.h"
#include "Skin/SkinData.h"
#include "Core/General.h"
#include "Core/ChartScanner.h"
#include "File/ChartLoader.h"
#include "File/FindAllTJA.h"
#include "Core/text.h"

#include <unordered_set>

namespace SkinLayout {

	Vector2d ScrollField{ 325, 254 - 65 };
	int NoteSize = 128;

};
namespace fs = std::filesystem;


PlayScene::PlayScene() {

	//	ChartLoad::load(u8"(Songs/続・〆ドレー2000/続・〆ドレー2000.tja)", CD, CourseType::Oni);
	//	ChartLoad::load("Songs/シャイニングスター/シャイニングスター.tja", CD, CourseType::Oni);
	debug = "走査対象のパス: " + fs::absolute( "Songs" ).string(); // 絶対パスに変換して出力
	tempTjaPath = FindAllTjaFiles( "Songs" );
	ChartLoad::load( tempTjaPath[4].c_str(), CD, CourseType::Oni );
		//CD.loadSong("Resource/Debug/カンケーガール.mp3", 185.0, 4.2);
	double soundVol = 0.8;
	ChangeVolumeSoundMem( 255 * soundVol, CD.songData.songHandle.handle );
	/*
	for (int i = 0; i < 100; i++) {
		NoteType rndNote = static_cast<NoteType>(GetRand(1));
		CD.notes.push_back({ 158 , (long long)(240.0 / CD.bpm * 1000000.0) * i, rndNote});
	}
	*/
	//extern ChartScanner CD;
	CD.notes;


}


void PlayScene::Update() {

	this->Input();
	CD.Update();


}


void PlayScene::Draw() {




	DrawGraph( 0, 0, Skin::GetTexture( "play/bg" ).handle, true );

	// レーン
	DrawGraph( SkinLayout::ScrollField.x, SkinLayout::ScrollField.y, Skin::GetTexture( "play/ScrollField/bg" ).handle, true );

	DrawExtendGraph( SkinLayout::ScrollField.x,
					 SkinLayout::ScrollField.y,
					 SkinLayout::ScrollField.x + SkinLayout::NoteSize,
					 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
					 Skin::GetTexture( Skin::GetNoteImageKey( NoteType::Judge, false ) ).handle,
					 true );

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
		return ((CD.noteRelativeTime( note.idx ) / 1000000.0) / (240.0 / note.bpm)) * 900.0 * note.scroll + SkinLayout::ScrollField.x;
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
		long long rel = CD.noteRelativeTime( note.idx );
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
	const size_t notesIndex = CD.nextNoteIndex;

	std::unordered_set<size_t> drawnRolls;
	for ( size_t i = CD.notes.size(); i-- > 0; ) {
		const auto& note = CD.notes[i];
		noteRelativeTime = CD.noteRelativeTime( CD.notes.size() - 1 - index );
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

				const Note& rollHead = isRollHead ? note : CD.notes[note.pairRollIndex];
				const Note& rollTail = !isRollHead ? note : CD.notes[note.pairRollIndex];


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
	CD.notes; // デバッグで内部数値を確認する用


	int winx, winy;
	GetWindowSize( &winx, &winy );




	//////
	if ( lastRollIdx != SIZE_MAX ) {
		DrawFormatString2Right( winx,
								0,
								GetColor( 255, 255, 255 ),
								std::to_string( CD.notes[lastRollIdx].rollHitCount )
		);
	}

	if ( CD.nextNoteIndex < CD.notes.size() ) {
		DrawFormatString2Right( winx,
								16,
								GetColor( 255, 255, 255 ),
								std::to_string( static_cast<int>( CD.notes[CD.nextNoteIndex].type ) )
		);

	
	}

	std::string s;
	for (const auto& note : CD.notes) {
		if ( note.type != NoteType::BalloonHead ) continue;
		s = "Balloon ID: " + std::to_string(note.balloonId) + ", Required: " + std::to_string(note.requiredHits) + ", Hit: " + std::to_string(note.balloonHitCount);
		break;
	}
	DrawFormatString2Right( winx,
								32,
								GetColor( 0, 0, 0 ),
								s );

	//DrawExtendGraph( 0,
	//			 0,
	//			 winx,
	//			 winy,
	//			 Skin::GetTexture( Skin::GetRollImageKey( Skin::RollPart::Head, false ) ).handle,
	//			 true );


	std::string str = "";
	int strW = 0;
	int debugstrY = 0;
	std::string fps = std::to_string( FPS::getFps() );
	strW = GetDrawFormatStringWidth( fps.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), fps.c_str() );
	debugstrY += 16;


	std::string songhandle = std::to_string( CD.songData.songHandle.handle );
	strW = GetDrawFormatStringWidth( songhandle.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), songhandle.c_str() );
	debugstrY += 16;
	////
	str = debug;
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "[0]: " + tempTjaPath[0];
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "[1]: " + tempTjaPath[1];
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "[2]: " + tempTjaPath[2];
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "[3]: " + tempTjaPath[3];
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;


	str = "Songs/シャイニングスター/シャイニングスター.tja"; // 自分で、"/"だけを使って、ハードコードする
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), "%s", str.c_str() );
	debugstrY += 16;
	// tempTjaPathなど、これまでの複雑な経路を、一旦すべて無視する
	DrawFormatString( 0, 100, GetColor( 255, 255, 255 ), "シャイニングスター" ); // 直接、ハードコードで試す
	////
	str = "path: " + CD.tjaPath;
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;
	////
	str = "TITLE: " + CD.Title;
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "SUBTITLE: " + CD.subTitle;
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "BPM: " + std::to_string( CD.bpm );
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	str = "LEVEL: " + std::to_string( CD.level );
	strW = GetDrawFormatStringWidth( str.c_str() );
	DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
	DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), str.c_str() );
	debugstrY += 16;

	extern std::vector<ChartScanner::ChartFile> ChartList;
	for ( const auto& chartfile : ChartList ) {
		std::string path = chartfile.ChartPath.string();
		strW = GetDrawFormatStringWidth( path.c_str() );
		DrawBox( 0, debugstrY, strW, debugstrY + 16, GetColor( 0, 0, 0 ), true );
		DrawFormatString( 0, debugstrY, GetColor( 255, 255, 255 ), path.c_str() );
		debugstrY += 16;
	}

}

void PlayScene::Input() {

	CD.Input();
}