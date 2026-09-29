#include "Dxlib.h"
#include "ChartLoader.h"

#include "Core/ChartData.h"
#include "Core/General.h"
#include "File/FileUtil.h"

#include "CharCode.h"
#include <string>
#include <vector>
#include <cassert>
#include <filesystem>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <cctype> // isdigit
#include <map>
#include <cstdint> // SIZE_MAX

/// 余裕があればSHIFT-JISの文字コードにも対応させる。とりあえずUTF8のみに対応させ、譜面ファイル側でUTF8に変換してもらう。


namespace fs = std::filesystem;

namespace {
	class ChartLoader
	{

//CourseType targetCourse = CourseType::Oni;	// 難易度コース: easy = 0, normal = 1, hard = 2, oni = 3, master = 4;
//double level = 0.0;		// 難易度レベル:

//std::string measurenotestr = "";	// 小節のノーツを格納する文字列。カンマが見つかるまでのノーツを格納し、1小節分として扱うためのバッファ
//long long measureStartTime = 0;		// 小節の開始時間(us)
//double nowMeasureScale = 1.0;		// 現在の小節のスケール(拍子) 1.0 = 4分音符, 0.5 = 8分音符, 2.0 = 2分音符
//double ps.measureBPM = 120.0;		// 現在の小節のBPM
//double nowScroll = 1.0;				// 現在のスクロール速度
//bool BarlineFlag = false;			// 小節線の有無を示すフラグ

//size_t rollCount = 0;				// ロールidの蓄積
//size_t lastRollId = SIZE_MAX;		// 最後のロールid。SIZE_MAXでペア処理終了状態.



		/// nowValue (パースに使用する変数群)
		struct ParseContexts
		{

			CourseType targetCourse = CourseType::Oni;	// 難易度コース: easy = 0, normal = 1, hard = 2, oni = 3, master = 4;

			double level = 0.0;							// 難易度レベル:

			std::string measureNoteStr = "";			// 小節のノーツを格納する文字列。カンマが見つかるまでのノーツを格納し、1小節分として扱うためのバッファ
			long long measureStartTime = 0;				// 小節の開始時間(us)

			double measureScale = 1.0;					// 現在の小節のスケール(拍子) 1.0 = 4分音符, 0.5 = 8分音符, 2.0 = 2分音符
			double measureBPM = -1;						// 現在の小節のBPM
			double Scroll = 1.0;						// 現在のスクロール速度
			bool BarlineFlag = false;					// 小節線の有無を示すフラグ

			size_t rollCount = 0;						// ロールidの蓄積
			size_t lastRollId = SIZE_MAX;				// 最後のロールid。SIZE_MAXでペア処理終了状態.

		};



		/// デバッグ用
		struct DebugInfo
		{
			size_t LastMeasureNoteSize;
		};



		///// コマンドイベントクラス 
		struct commandEvent
		{
			enum class CommandType
			{
				BPMCHANGE,
				MEASURE,
				BARLINEOFF,
				BARLINEON,
				SCROLL
			};


			CommandType type;
			double value;	// BPMCHANGE, SCROLL の場合は値を格納する。MEASUREの場合は分子/分母の比率を格納する。BARLINEOFF, BARLINEONの場合は使用しない。

		};

		std::map<int, std::vector<commandEvent>> commandEventsMap;	// ノーツ番号をキーとして、コマンドイベントのリストを格納するマップ。ノーツ番号は小節内のノーツインデックスで、0から始まる。


		///
		/// 行取得系、行編集系関数群、trim, removeCommentなどなど
		///

		std::string removeComment( const std::string& str ) {	// 行のコメントを削除する
			size_t commentPos = str.find( "//" );
			if ( commentPos != std::string::npos ) {
				return str.substr( 0, commentPos );
			}
			return str;
		}


		std::string getValueAfterColon( const std::string& str ) {
			return string_util::trimWhitespace( str.substr( str.find( ':' ) + 1 ) );
		}

		std::string getValueAfterWhitespace( const std::string& str ) {
			return string_util::trimWhitespace( str.substr( str.find( ' ' ) ) );
		}


		// データ取得系

		void getHeaderData( std::string& line, ChartData& cd, ParseContexts& ps ) {

			if ( line.starts_with( "TITLE" ) ) {

				cd.Title = getValueAfterColon( line );

			}
			else if ( line.starts_with( "SUBTITLE" ) ) {

				cd.subTitle = getValueAfterColon( line );

			}
			else if ( line.starts_with( "BPM" ) ) {

				cd.bpm = std::stod( getValueAfterColon( line ) );
				ps.measureBPM = cd.bpm; // 現在の小節のBPMを設定

			}
			else if ( line.starts_with( "OFFSET" ) ) {

				cd.offset = -std::stod( getValueAfterColon( line ) );

			}
			else if ( line.starts_with( "WAVE" ) ) {

				cd.songPath = getValueAfterColon( line );

			}
			else if ( line.starts_with( "DEMOSTART" ) ) {

				cd.demoStart = std::stod( getValueAfterColon( line ) );

			}
		}

		void getCourseData( const std::string& line, ChartData& cd, ParseContexts& ps ) {

			if ( line.starts_with( "LEVEL" ) ) {

				ps.level = std::stod( getValueAfterColon( line ) );
				cd.level = ps.level;

			}
			else if ( line.starts_with( "BALLOON" ) ) {

				std::vector<std::string> balloonsStr = string_util::split( (getValueAfterColon( line )) );
				for ( std::string& val : balloonsStr ) {
					val = (val == "") ? "0" : val; // 空文字列の場合は0に置き換える
					int temp = std::stoi( val );
					size_t num = static_cast<size_t>(std::clamp( temp, 0, 999 ));
					cd.balloon.push_back( num );
				}

			}

		}


		void loadSong( ChartData& cd ) {
			fs::path tja( cd.tjaPath );
			fs::path folder = tja.parent_path();

			std::string songfullpath = (folder / cd.songPath).string();
			cd.loadSong( songfullpath.c_str() );
		}

		CourseType parseCourseType( const std::string& str ) {	// 譜面のCOURSE: の項目を受け取り、対応するCourseTypeを返す
			static const std::unordered_map<std::string, CourseType> courseMap = {
				{ "easy",		CourseType::Easy },
				{ "0",			CourseType::Easy },
				{ "normal",		CourseType::Normal },
				{ "1",			CourseType::Normal },
				{ "hard",		CourseType::Hard },
				{ "2",			CourseType::Hard },
				{ "oni",			CourseType::Oni },
				{ "3",			CourseType::Oni },
				{ "expert",		CourseType::Oni },
				{ "edit",		CourseType::InnerOni },
				{ "inner",		CourseType::InnerOni },
				{ "inneroni",	CourseType::InnerOni },
				{ "4",			CourseType::InnerOni },
				{ "insane",		CourseType::InnerOni },

			};
			std::string lowerValue = str;
			std::transform( lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower ); // 小文字化
			auto result = courseMap.find( lowerValue );
			if ( result != courseMap.end() ) {
				return result->second;
			}


			assert( false && "難易度が見つかりません。" );
			return CourseType::Oni;
		}

		void getNoteData( const std::string& line, ChartData& cd, ParseContexts& ps ) {	// ノーツデータおよびコマンドの解析処理


			/// コマンド処理
			if ( line.starts_with( "#" ) ) {

				parseCommand( line, cd, ps);

			}
			else {
				// ノーツ

				for ( const char c : line ) {

					if ( std::isdigit( static_cast<unsigned char>(c) ) ) {	// 数字の場合はノーツとして扱う
						//OutputDebugString(("Found digit char:" + std::string(1, c) + "\n").c_str());
						ps.measureNoteStr += c;
					}
					else if ( c == ',' ) {
					   // カンマが見つかったら、現在のノーツを解析して追加する
					   // デバッグ用にノーツ数を表示
					   //OutputDebugString(("Measure notes size:" + std::to_string(measurenotestr.size()) + ", Str: " + measurenotestr + "\n").c_str());
						parseMeasureNoteStr( cd, ps );
					}

				}




			}



		}

		void parseMeasureNoteStr( ChartData& cd, ParseContexts& ps) {	// measurenotestrに格納された1小節分のノーツを解析し、cd.notesに追加する
			// ここでmeasurenotestrを解析し、cd.notesにノーツを追加する処理を実装する
			// 例: "001002003" -> ノーツの種類とタイミングを計算してcd.notesに追加する
			// 解析後、measurenotestrをクリアする

			// todo: コマンドイベントでBPMや拍子を変更する場合、measureDurationの値がおかしくなるため、measureDuraiton/noteCountの計算から方針を変える必要がある。
			// 小節の半分地点でMEASURE 2/1が実行された場合、小節の後半のノーツの間隔が半分になる。
			// そのため、ノーツの絶対座標計算にmeasureStartTimeを使用するのではなく、直前のノーツからの相対座標計算を行う必要がある。 ←ダメかも。先頭ノーツの場合インデックスがマイナスになる。
			// noteIntervalを蓄積させることでその小節内での相対座標を割り出す

			auto apllyCommandEvent = [&]( int noteIndex ) {
				// コマンドイベントの適用
				for ( const auto& event : commandEventsMap[noteIndex] ) {
					using enum commandEvent::CommandType;
					switch ( event.type ) {
						case BPMCHANGE:
							ps.measureBPM = event.value;
							break;
						case MEASURE:
							ps.measureScale = event.value;
							break;
						case BARLINEOFF:
							ps.BarlineFlag = false;
							break;
						case BARLINEON:
							ps.BarlineFlag = true;
							break;
						case SCROLL:
							ps.Scroll = event.value;
							break;
					}
				}
			};





			long long measureDuration = static_cast<long long>((240.0 / ps.measureBPM) * 1000000.0 * ps.measureScale); // 小節の長さを計算する

			int noteCount = ps.measureNoteStr.size(); // 小節内のノーツ数
			long long noteInterval; // ノーツ間の時間間隔を計算する



			if ( noteCount == 0 ) {
				apllyCommandEvent( 0 ); // 小節内にノーツがない場合は、最初のノーツにコマンドイベントを適用する (これが無い場合スキップされてしまう)
				measureDuration = static_cast<long long>((240.0 / ps.measureBPM) * 1000000.0 * ps.measureScale); // 小節の長さを計算する
				ps.measureStartTime += measureDuration;
				return;
			}

			long long measureNoteRelativeTime = 0;



			/// 連打処理中にノーツが来たとき、連打尾ノーツを生成する
			auto insertRollTail = [&]( const Note& note ) {
				if ( ps.lastRollId != SIZE_MAX ) {
					// 5または6の後に8を介さずにノーツを生成しようとしたときに呼ばれる
					Note rollTail;
					{
						rollTail = note;
						rollTail.type = NoteType::RollTail;
						rollTail.absTime -= noteInterval;
						rollTail.rollId = ps.lastRollId;
					}

					cd.notes.push_back( rollTail );
				}
			};
			///

			for ( int i = 0; i < noteCount; ++i ) {
				// ノーツの解析と追加処理


				/// コマンドイベントの適用
				apllyCommandEvent( i );


				bool skip = false; // ノーツがNoneとして扱われたとき、また小節線関係で存在する必要のないとき、ノーツを追加しないようにするためのフラグ

				Note note;

				note.hasBarline = ((i == 0) && ps.BarlineFlag); // 最初のノーツに小節線を付与する

				measureDuration = static_cast<long long>((240.0 / ps.measureBPM) * 1000000.0 * ps.measureScale); // 小節の長さを計算する
				noteInterval = measureDuration / noteCount;
				note.absTime = ps.measureStartTime + measureNoteRelativeTime; // ノーツの絶対時間を計算する
				note.bpm = ps.measureBPM; // ノーツのBPMを設定する
				note.scroll = ps.Scroll; // ノーツのスクロール速度を設定する

				char noteStr = ps.measureNoteStr[i];
				note.isBig = std::string( "346" ).find( noteStr ) != std::string::npos; // find関数を活用してこれで3, 4, 6を条件としてtrue/falseを分岐できる！！！かしこい！！！

				switch ( noteStr ) {	// charごとにノーツを分岐
					case '0':
						skip = (i != 0);		// NoteTypeがNoneかつ、先頭ではないとき、必要ないのでskipをtrue
						note.type = NoteType::None;
						break;
					case '1':
					case '3':
						insertRollTail( note );
						note.type = NoteType::Don;
						break;
					case '2':
					case '4':
						insertRollTail( note );
						note.type = NoteType::Katsu;
						break;
					case '5':
					case '6':
						insertRollTail( note );
						note.type = NoteType::RollHead;
						note.rollId = ps.rollCount++;
						ps.lastRollId = note.rollId;
						break;
					case '8':

						skip = (i != 0 && ps.lastRollId == SIZE_MAX);	// 先頭ではないかつ、連打のペアがいない場合、存在価値が無い
						note.type = NoteType::RollTail;
						note.rollId = ps.lastRollId;
						ps.lastRollId = SIZE_MAX;
						break;
					default:
						//assert(false && "不明なノーツタイプです。");
						OutputDebugString( ("Unknown note type: " + std::string( 1, noteStr ) + "\n").c_str() );
						note.type = NoteType::None; // 連打やくす玉は未実装につき、とりあえずNoneとして扱う
						break;
				}

				if ( !skip ) cd.notes.push_back( note );
				measureNoteRelativeTime += noteInterval; // 次のノーツの相対時間を計算する
			}

			ps.measureStartTime += measureNoteRelativeTime; // 次の小節の開始時間を計算する (最終的なノーツの相対座標が次の小節の開始位置になる....はず)
			ps.measureNoteStr.clear();
			commandEventsMap.clear(); // 小節が終わったらコマンドイベントをクリアする
		}


		void parseCommand( const std::string& command, ChartData& cd, ParseContexts& ps ) {	// コマンドの処理

			auto addCommandEvent = [&]( commandEvent::CommandType type, double value ) {
				size_t noteIndex = static_cast<size_t>(ps.measureNoteStr.size()); // 現在のノーツ数を取得 (ノーツのインデックス)
				commandEvent event{ type, value };
				commandEventsMap[noteIndex].push_back( event );
			};



			if ( command.starts_with( "#BPMCHANGE" ) ) {		// #BPMCHANGE 185 → ps.measureBPM = 185
				std::string bpmStr = getValueAfterWhitespace( command );
				double bpm = std::stod( bpmStr );
				addCommandEvent( commandEvent::CommandType::BPMCHANGE, std::stod( bpmStr ) );
			}
			else if ( command.starts_with( "#MEASURE" ) ) {		// #MEASURE 4/4 → nowMeasureScale = 1.0, #MEASURE 3/4 → nowMeasureScale = 3/4 = 0.75
				std::string measureStr = getValueAfterWhitespace( command );
				size_t slashPos = measureStr.find( '/' );
				if ( slashPos != std::string::npos ) {
					double numerator = std::stod( measureStr.substr( 0, slashPos ) );
					double denominator = std::stod( measureStr.substr( slashPos + 1 ) );
					addCommandEvent( commandEvent::CommandType::MEASURE, numerator / denominator );
				}
			}
			else if ( command.starts_with( "#BARLINEOFF" ) ) {
				ps.BarlineFlag = false;
				addCommandEvent( commandEvent::CommandType::BARLINEOFF, 0 );
			}
			else if ( command.starts_with( "#BARLINEON" ) ) {
				ps.BarlineFlag = true;
				addCommandEvent( commandEvent::CommandType::BARLINEON, 0 );
			}
			else if ( command.starts_with( "#SCROLL" ) ) {
				std::string scrollStr = getValueAfterWhitespace( command );
				addCommandEvent( commandEvent::CommandType::SCROLL, std::stod( scrollStr ) );
			}


			if ( command.starts_with( "#DEBUG" ) ) {
				if ( command.find( "stop" ) != std::string::npos ) {
					assert( false && "DEBUG: stop command found" );	// デバッグ用に譜面の読み込みを停止する。これにより、譜面の読み込みが途中で止まるので、譜面の解析が正しく行われているかを確認できる。
				}
				else if ( command.find( "out" ) != std::string::npos ) {
					OutputDebugString( ("DEBUG: " + ps.measureNoteStr+ "\n" + \
										 std::to_string( ps.measureScale ) + "\n" + \
										 std::to_string( commandEventsMap[0].size() ) + "\n"
										 ).c_str() );	// デバッグ用にここに書いた内容を出力する。ノーツリストやnowScrollなどを確認できる

				}
			}
		}


	public:
		int LoadChart( const char* path, ChartData& cd, CourseType _course ) {

			ParseContexts ps;


			cd.tjaPath = path;

			ps.targetCourse = _course;
			cd.course = _course;
			std::vector<std::string> allLines;


			allLines = file_util::getAllLines( path ); // ファイルのテキスト情報をストリングに書き出し


			bool noteDataSection = false;	// ノーツデータブロックかどうか (#STARTから#ENDの間)
			bool foundCourseData = false;		// 検索対象のコースを発見したかどうか
			bool hasCourse = false;		// コース指定がない譜面ファイルかどうか (COURSE:の記述がない場合は、すべてのコースに適用される)

			std::string noteBuffer = "";	// カンマが見つかるまでのノーツを格納し、1小節分として扱うためのバッファ



			for ( const auto& rawline : allLines ) {
				std::string removedCommentsLine = removeComment( rawline );
				std::string line = string_util::trimWhitespace( removedCommentsLine );



				if ( line.starts_with( "COURSE:" ) ) {
					hasCourse = true;
				}

				if ( line.starts_with( "#START" ) ) {
					if ( !hasCourse || foundCourseData ) {
						noteDataSection = true;
					}
					continue;
				}
				else if ( line.starts_with( "#END" ) && foundCourseData ) {
					noteDataSection = false;
					break;
				}


				if ( noteDataSection ) {
					// ノーツデータ
					getNoteData( line, cd, ps );
					continue;
				}
				else {
					if ( foundCourseData ) {
						// コースデータ
						getCourseData( line, cd, ps );
						continue;
					}
					else if ( line.starts_with( "COURSE:" ) ) {
						CourseType _course = parseCourseType( getValueAfterColon( line ) );
						if ( _course == ps.targetCourse ) {
							foundCourseData = true;
						}
						continue;
					}
				}


				getHeaderData( line, cd, ps );	//譜面のヘッダー情報取得(TITLEやSUBTITLE, BPMなど)
			}


			//// 譜面データのソート
			std::stable_sort( cd.notes.begin(), cd.notes.end(), []( const Note& a, const Note& b ) {return a.absTime < b.absTime; } );


			// 連打の再接続とノーツのidxを設定

			for ( size_t i = 0; i < cd.notes.size(); i++) {
				Note& note = cd.notes[i];
				note.idx = i;
				if ( note.type != NoteType::RollHead ) continue;
				Note& rollHead = note;

				// 手法1
				for ( size_t j = 0; j < cd.notes.size(); j++) {
					Note& rollTail = cd.notes[j];
					if ( rollTail.type != NoteType::RollTail ) continue;
					if ( rollTail.rollId != rollHead.rollId ) continue;

					rollHead.pairRollIndex = j;
					rollTail.pairRollIndex = i;
				}


			}



			//
			loadSong( cd );

			for ( Note& note : cd.notes ) {
				if ( !(note.type == NoteType::RollHead || note.type == NoteType::RollTail) ) continue;
				OutputDebugString( ("Type: " + std::to_string(static_cast<int>(note.type)) + ", RollID: " + std::to_string(note.rollId) + "\n").c_str());
			}

			for ( Note& note : cd.notes ) {
				OutputDebugString( ("Type: " + std::to_string( static_cast<int>(note.type) ) + ", BPM: " + std::to_string( note.bpm ) + ", SCROLL: " + std::to_string( note.scroll ) + "\n").c_str() );
			}

			return 0;
		}
	};

	ChartLoader cl;


}



namespace ChartLoad {
	int load( const char* path, ChartData& cd, CourseType course ) { // 譜面の読み込みに失敗した場合、-1を返し、選曲画面に戻す(Chartloadingシーン)

		return cl.LoadChart( path, cd, course );
	}
}

