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
	class ChartLoader {

		CourseType targetCourse = CourseType::Oni;	// 難易度コース: easy = 0, normal = 1, hard = 2, oni = 3, master = 4;
		double level = 0.0;		// 難易度レベル:

		std::string measureNoteStr = "";	// 小節のノーツを格納する文字列。カンマが見つかるまでのノーツを格納し、1小節分として扱うためのバッファ
		long long measureStartTime = 0;		// 小節の開始時間(us)
		double nowMeasureScale = 1.0;		// 現在の小節のスケール(拍子) 1.0 = 4分音符, 0.5 = 8分音符, 2.0 = 2分音符
		double nowMeasureBPM = 120.0;		// 現在の小節のBPM
		double nowScroll = 1.0;				// 現在のスクロール速度
		bool BarlineFlag = false;			// 小節線の有無を示すフラグ

		

		/// デバッグ用
		struct DebugInfo {
			size_t LastMeasureNoteSize;
		};



		///// コマンドイベントクラス 
		struct commandEvent {
			enum class CommandType {
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

		std::string removeComment(const std::string& str) {	// 行のコメントを削除する
			size_t commentPos = str.find("//");
			if (commentPos != std::string::npos) {
				return str.substr(0, commentPos);
			}
			return str;
		}


		std::string getValueAfterColon(const std::string& str) {
			return string_util::trimWhitespace(str.substr(str.find(':') + 1));
		}

		std::string getValueAfterWhitespace(const std::string& str) {
			return string_util::trimWhitespace(str.substr(str.find(' ')));
		}


		// データ取得系

		void getHeaderData(std::string& line, ChartData& cd) {

			if (line.starts_with("TITLE")) {

				cd.Title = getValueAfterColon(line);

			}
			else if (line.starts_with("SUBTITLE")) {

				cd.subTitle = getValueAfterColon(line);

			}
			else if (line.starts_with("BPM")) {

				cd.bpm = std::stod(getValueAfterColon(line));
				nowMeasureBPM = cd.bpm; // 現在の小節のBPMを設定

			}
			else if (line.starts_with("OFFSET")) {

				cd.offset = -std::stod(getValueAfterColon(line));

			}
			else if (line.starts_with("WAVE")) {

				cd.songPath = getValueAfterColon(line);

			}
			else if (line.starts_with("DEMOSTART")) {

				cd.demoStart = std::stod(getValueAfterColon(line));

			}
		}
		
		void getCourseData(const std::string& line, ChartData& cd) {

			if (line.starts_with("LEVEL")) {

				cd.level = std::stod(getValueAfterColon(line));

			}
			else if (line.starts_with("BALLOON")) {

				std::vector<std::string> balloonsStr = string_util::split((getValueAfterColon(line)));
				for (std::string& val : balloonsStr) {
					val = (val == "") ? "0" : val; // 空文字列の場合は0に置き換える
					int temp = std::stoi(val);
					size_t num = static_cast<size_t>(std::clamp(temp, 0, 999));
					cd.balloon.push_back(num);
				}

			}

		}


		void loadSong(ChartData& cd) {
			fs::path tja(cd.tjaPath);
			fs::path folder = tja.parent_path();

			std::string songfullpath = (folder / cd.songPath).string();
			cd.loadSong(songfullpath.c_str());
		}

		CourseType parseCourseType(const std::string& str) {	// 譜面のCOURSE: の項目を受け取り、対応するCourseTypeを返す
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
			std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower); // 小文字化
			auto result = courseMap.find(lowerValue);
			if (result != courseMap.end()) {
				return result->second;
			}
			

			assert( false && "難易度が見つかりません。");
			return CourseType::Oni;
		}

		void getNoteData(const std::string& line, ChartData& cd) {	// ノーツデータおよびコマンドの解析処理
			

			/// コマンド処理
			if (line.starts_with("#")) {

				parseCommand(line, cd);

			}
			else {
				// ノーツ

				for (const char c : line) {

					if(std::isdigit(static_cast<unsigned char>(c))) {	// 数字の場合はノーツとして扱う
						//OutputDebugString(("Found digit char:" + std::string(1, c) + "\n").c_str());
						measureNoteStr += c;
					}else if(c == ',') {
						// カンマが見つかったら、現在のノーツを解析して追加する
						// デバッグ用にノーツ数を表示
						//OutputDebugString(("Measure notes size:" + std::to_string(measureNoteStr.size()) + ", Str: " + measureNoteStr + "\n").c_str());
						parseMeasureNoteStr(cd);
					}

				}




			}

		}

		void parseMeasureNoteStr(ChartData& cd) {	// measureNoteStrに格納された1小節分のノーツを解析し、cd.notesに追加する
			// ここでmeasureNoteStrを解析し、cd.notesにノーツを追加する処理を実装する
			// 例: "001002003" -> ノーツの種類とタイミングを計算してcd.notesに追加する
			// 解析後、measureNoteStrをクリアする
			
			// todo: コマンドイベントでBPMや拍子を変更する場合、measureDurationの値がおかしくなるため、measureDuraiton/noteCountの計算から方針を変える必要がある。
			// 小節の半分地点でMEASURE 2/1が実行された場合、小節の後半のノーツの間隔が半分になる。
			// そのため、ノーツの絶対座標計算にmeasureStartTimeを使用するのではなく、直前のノーツからの相対座標計算を行う必要がある。 ←ダメかも。先頭ノーツの場合インデックスがマイナスになる。
			// noteIntervalを蓄積させることでその小節内での相対座標を割り出す

			auto apllyCommandEvent = [&](int noteIndex) {
				// コマンドイベントの適用
				for (const auto& event : commandEventsMap[noteIndex]) {
					using enum commandEvent::CommandType;
					switch (event.type) {
					case BPMCHANGE:
						nowMeasureBPM = event.value;
						break;
					case MEASURE:
						nowMeasureScale = event.value;
						break;
					case BARLINEOFF:
						BarlineFlag = false;
						break;
					case BARLINEON:
						BarlineFlag = true;
						break;
					case SCROLL:
						nowScroll = event.value;
						break;
					}
				}
				};





			long long measureDuration = static_cast<long long>((240.0 / nowMeasureBPM) * 1000000.0 * nowMeasureScale); // 小節の長さを計算する

			int noteCount = measureNoteStr.size(); // 小節内のノーツ数
			long long noteInterval; // ノーツ間の時間間隔を計算する
			


			if (noteCount == 0) {
				apllyCommandEvent(0); // 小節内にノーツがない場合は、最初のノーツにコマンドイベントを適用する (これが無い場合スキップされてしまう)
				measureDuration = static_cast<long long>((240.0 / nowMeasureBPM) * 1000000.0 * nowMeasureScale); // 小節の長さを計算する
				measureStartTime += measureDuration;
				return;
			}

			long long measureNoteRelativeTime = 0;
			for (int i = 0; i < noteCount; ++i) {
				// ノーツの解析と追加処理


				/// コマンドイベントの適用
				apllyCommandEvent(i);
				
				
				Note note;

				if (i == 0 && BarlineFlag) note.hasBarline = true; // 最初のノーツに小節線を付与する
				
				measureDuration = static_cast<long long>((240.0 / nowMeasureBPM) * 1000000.0 * nowMeasureScale); // 小節の長さを計算する
				noteInterval = measureDuration / noteCount;
				note.absTime = measureStartTime + measureNoteRelativeTime; // ノーツの絶対時間を計算する
				note.bpm = nowMeasureBPM; // ノーツのBPMを設定する
				note.scroll = nowScroll; // ノーツのスクロール速度を設定する

				char noteStr = measureNoteStr[i];
				note.isBig = std::string("346").find(noteStr) != std::string::npos; // find関数を活用してこれで3, 4, 6を条件としてtrue/falseを分岐できる！！！かしこい！！！
				switch (measureNoteStr[i]) {
					case '0':
						note.type = NoteType::None;
						break;
					case '1':
					case '3':
						note.type = NoteType::Don;
						break;
					case '2':
					case '4':
						note.type = NoteType::Katsu;
						break;
					case '5':
					case '6':
						note.type = NoteType::RollHead;
						break;
					case '8':
						note.type = NoteType::RollTail;
						break;
					default:
						//assert(false && "不明なノーツタイプです。");
						OutputDebugString(("Unknown note type: " + std::string(1, measureNoteStr[i]) + "\n").c_str());
						note.type = NoteType::None; // 連打やくす玉は未実装につき、とりあえずNoneとして扱う
						break;
				}

				cd.notes.push_back(note);
				measureNoteRelativeTime += noteInterval; // 次のノーツの相対時間を計算する
			}

			measureStartTime += measureNoteRelativeTime; // 次の小節の開始時間を計算する (最終的なノーツの相対座標が次の小節の開始位置になる....はず)
			measureNoteStr.clear();
			commandEventsMap.clear(); // 小節が終わったらコマンドイベントをクリアする
		}


		void parseCommand(const std::string& command, ChartData& cd) {	// コマンドの処理

			auto addCommandEvent = [&](commandEvent::CommandType type, double value) {
				size_t noteIndex = static_cast<size_t>(measureNoteStr.size()); // 現在のノーツ数を取得 (ノーツのインデックス)
				commandEvent event{ type, value };
				commandEventsMap[noteIndex].push_back(event);
				};



			if (command.starts_with("#BPMCHANGE")) {		// #BPMCHANGE 185 → nowMeasureBPM = 185
				std::string bpmStr = getValueAfterWhitespace(command);
				addCommandEvent(commandEvent::CommandType::BPMCHANGE, std::stod(bpmStr));
			}
			else if (command.starts_with("#MEASURE")) {		// #MEASURE 4/4 → nowMeasureScale = 1.0, #MEASURE 3/4 → nowMeasureScale = 3/4 = 0.75
				std::string measureStr = getValueAfterWhitespace(command);
				size_t slashPos = measureStr.find('/');
				if (slashPos != std::string::npos) {
					double numerator = std::stod(measureStr.substr(0, slashPos));
					double denominator = std::stod(measureStr.substr(slashPos + 1));
					addCommandEvent(commandEvent::CommandType::MEASURE, numerator / denominator);
				}
			}
			else if (command.starts_with("#BARLINEOFF")) {
				BarlineFlag = false;
				addCommandEvent(commandEvent::CommandType::BARLINEOFF, 0);
			}
			else if (command.starts_with("#BARLINEON")) {
				BarlineFlag = true;
				addCommandEvent(commandEvent::CommandType::BARLINEON, 0);
			}
			else if (command.starts_with("#SCROLL")) {
				std::string scrollStr = getValueAfterWhitespace(command);
				addCommandEvent(commandEvent::CommandType::SCROLL, std::stod(scrollStr));
			}


			if (command.starts_with("#DEBUG")){
				if (command.find("stop") != std::string::npos) {
					assert(false && "DEBUG: stop command found");	// デバッグ用に譜面の読み込みを停止する。これにより、譜面の読み込みが途中で止まるので、譜面の解析が正しく行われているかを確認できる。
				}
				else if(command.find("out") != std::string::npos) {
					OutputDebugString(("DEBUG: " +	measureNoteStr + "\n" + \
													std::to_string(nowMeasureScale) + "\n" + \
													std::to_string(commandEventsMap[0].size()) + "\n"
						).c_str());	// デバッグ用にここに書いた内容を出力する。ノーツリストやnowScrollなどを確認できる

				}
			}
		}


	public:
		int LoadChart(const char* path, ChartData& cd, CourseType _course) {

			cd.tjaPath = path;

			targetCourse = _course;
			cd.course = _course;
			std::vector<std::string> allLines;


			allLines = file_util::getAllLines(path); // ファイルのテキスト情報をストリングに書き出し


			bool noteDataSection = false;	// ノーツデータブロックかどうか (#STARTから#ENDの間)
			bool foundCourseData = false;		// 検索対象のコースを発見したかどうか
			bool hasCourse = false;		// コース指定がない譜面ファイルかどうか (COURSE:の記述がない場合は、すべてのコースに適用される)

			std::string noteBuffer = "";	// カンマが見つかるまでのノーツを格納し、1小節分として扱うためのバッファ


			std::vector<Note> TailNotes;
			Note LastRollHeadBuf;


			for (const auto& rawline : allLines) {
				std::string removedCommentsLine = removeComment(rawline);
				std::string line = string_util::trimWhitespace(removedCommentsLine);



				if (line.starts_with("COURSE:")) {
					hasCourse = true;
				}

				if (line.starts_with("#START")) {
					if (!hasCourse || foundCourseData) {
						noteDataSection = true;
					}
					continue;
				}
				else if (line.starts_with("#END") && foundCourseData) {
					noteDataSection = false;
					break;
				}


				if (noteDataSection) {
					// ノーツデータ
					getNoteData(line, cd);
					continue;
				}
				else {
					if (foundCourseData) {
						// コースデータ
						getCourseData(line, cd);
						continue;
					}
					else if (line.starts_with("COURSE:")) {
						CourseType _course = parseCourseType(getValueAfterColon(line));
						if (_course == targetCourse) {
							foundCourseData = true;
						}
						continue;
					}
				}

				
				getHeaderData(line, cd);	//譜面のヘッダー情報取得(TITLEやSUBTITLE, BPMなど)
			}


			//// 譜面データのソート
			std::stable_sort(cd.notes.begin(), cd.notes.end(), [](const Note& a, const Note& b) {return a.absTime < b.absTime;});


			loadSong(cd);

			return 0;
		}
	};

	ChartLoader cl;


}



namespace ChartLoad {
	int load(const char* path, ChartData& cd, CourseType course) { // 譜面の読み込みに失敗した場合、-1を返し、選曲画面に戻す(Chartloadingシーン)

		return cl.LoadChart(path, cd, course);
	}
}

