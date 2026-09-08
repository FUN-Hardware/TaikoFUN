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
#include <cctype> // isdisit

/// 余裕があればSHIFT-JISの文字コードにも対応させる。とりあえずUTF8のみに対応させ、譜面ファイル側でUTF8に変換してもらう。


namespace fs = std::filesystem;

namespace {
	class ChartLoader {

		CourseType targetCourse;	// 難易度コース: easy = 0, normal = 1, hard = 2, oni = 3, master = 4;
		double level;		// 難易度レベル:

		std::string measureNoteStr = "";	// 小節のノーツを格納する文字列。カンマが見つかるまでのノーツを格納し、1小節分として扱うためのバッファ
		long long measureStartTime = 0;		// 小節の開始時間(us)
		double nowMeasureScale = 1.0;		// 現在の小節のスケール(拍子) 1.0 = 4分音符, 0.5 = 8分音符, 2.0 = 2分音符
		double nowMeasureBPM = 120.0;		// 現在の小節のBPM
		double nowScroll = 1.0;				// 現在のスクロール速度
		bool BarlineFlag = false;			// 小節線の有無を示すフラグ


		std::vector<std::string> getAllLines(const char* filepath) {	// この関数は不要になりました。代わりにgetAllLines2を使うことにします。理由は、DxLibのFileRead系関数は、UTF8のBOM付きファイルを正しく読み込めないためです。BOM付きUTF8ファイルを正しく読み込むためには、std::ifstreamを使う必要があります。

			std::vector<std::string> result;
			
			int handle = FileRead_open(filepath);
			if (handle == -1) {
				assert(handle != -1 && "譜面ファイルが見つかりません");
				return result;
			}

			file_util::SetFileCharCode(filepath, handle); // 署名付きUTF8の場合、UTF8として、でない場合SHIFT-JISとしてファイルを扱う　ありがとうclaude	追記:manifestのおかげで不要になりました。
			// 
			const unsigned int MaxLinesLen = 1024;

			char line[MaxLinesLen];


			while (FileRead_eof(handle) == 0) {

				FileRead_gets(line, MaxLinesLen, handle);
				result.push_back(std::string(line));

			}

			FileRead_close(handle);

			return result;
		}
		

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
						measureNoteStr += c;
					}else if(c == ',') {
						// カンマが見つかったら、現在のノーツを解析して追加する
						parseMeasureNoteStr(cd);
					}

				}




			}

		}

		void parseMeasureNoteStr(ChartData& cd) {	// measureNoteStrに格納された1小節分のノーツを解析し、cd.notesに追加する
			// ここでmeasureNoteStrを解析し、cd.notesにノーツを追加する処理を実装する
			// 例: "001002003" -> ノーツの種類とタイミングを計算してcd.notesに追加する
			// 解析後、measureNoteStrをクリアする
			



			long long measureDuration = static_cast<long long>((240.0 / nowMeasureBPM) * 1000000.0 * nowMeasureScale); // 小節の長さを計算する

			if (measureNoteStr == "") {	// 小節内にノーツがない場合は、次の小節の開始時間を計算して終了する
				measureStartTime += measureDuration;
				return;
			}
			int noteCount = measureNoteStr.size(); // 小節内のノーツ数
			long long noteInterval = measureDuration / noteCount; // ノーツ間の時間間隔を計算する

			for (int i = 0; i < noteCount; ++i) {
				// ノーツの解析と追加処理

				Note note;

				if (i == 0) note.hasBarline = true; // 最初のノーツに小節線を付与する
				note.absTime = measureStartTime + i * noteInterval; // ノーツの絶対時間を計算する
				note.bpm = nowMeasureBPM; // ノーツのBPMを設定する
				note.scroll = nowScroll; // ノーツのスクロール速度を設定する
				switch (measureNoteStr[i]) {
					case '0':
						note.type = NoteType::None;
						break;
					case '1':
						note.type = NoteType::Don;
						break;
					case '2':
						note.type = NoteType::Katsu;
						break;
					case '3':
						note.type = NoteType::DonBig;
						break;
					case '4':
						note.type = NoteType::KatsuBig;
						break;
					default:
						//assert(false && "不明なノーツタイプです。");
						OutputDebugString(("Unknown note type: " + std::string(1, measureNoteStr[i]) + "\n").c_str());
						note.type = NoteType::None; // 連打やくす玉は未実装につき、とりあえずNoneとして扱う
						break;
				}

				cd.notes.push_back(note);
			}

			measureStartTime += measureDuration; // 次の小節の開始時間を計算する
			measureNoteStr.clear();
		}


		void parseCommand(const std::string& command, ChartData& cd) {	// コマンドの処理

			if (command.starts_with("#BPMCHANGE")) {		// #BPMCHANGE 185 → nowMeasureBPM = 185
				std::string bpmStr = getValueAfterWhitespace(command);
				nowMeasureBPM = std::stod(bpmStr);
			}
			else if (command.starts_with("#MEASURE")) {		// #MEASURE 4/4 → nowMeasureScale = 1.0, #MEASURE 3/4 → nowMeasureScale = 3/4 = 0.75
				std::string measureStr = getValueAfterWhitespace(command);
				size_t slashPos = measureStr.find('/');
				if (slashPos != std::string::npos) {
					double numerator = std::stod(measureStr.substr(0, slashPos));
					double denominator = std::stod(measureStr.substr(slashPos + 1));
					nowMeasureScale = numerator / denominator;
				}
			}
			else if (command.starts_with("#BARLINEOFF")) {
				BarlineFlag = false;
			}
			else if (command.starts_with("#BARLINEON")) {
				BarlineFlag = true;
			}
			else if (command.starts_with("#SCROLL")) {
				std::string scrollStr = getValueAfterWhitespace(command);
				nowScroll = std::stod(scrollStr);
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


			std::string noteBuffer = "";	// カンマが見つかるまでのノーツを格納し、1小節分として扱うためのバッファ

			for (const auto& rawline : allLines) {
				std::string removedCommentsLine = removeComment(rawline);
				std::string line = string_util::trimWhitespace(removedCommentsLine);

				if (line.starts_with("#START") && foundCourseData) {
					noteDataSection = true;
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

