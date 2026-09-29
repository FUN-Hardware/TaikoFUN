#pragma once

#include <vector>
#include "dxlib.h"
#include "Audio/SoundHandle.h"
#include "Time.h"
// ノーツデータと譜面データ

enum class NoteType {
	None,
	Don,
	Katsu,
	Judge,
	RollHead,
	RollTail,
	Balloon,
};


enum class JudgeType {
	GOOD,
	OK,
	BAD,
	MISS,
	ROLLHIT
};

enum class CourseType {
	Easy,
	Normal,
	Hard,
	Oni,
	InnerOni,

};

struct Note
{

	double bpm;				// ノーツの速度(流れる速さはこれに依存する)			
	long long absTime;		// ノーツの絶対座標　(曲オフセットからの相対時間)	ノーツが流れてくる位置
	NoteType type;				// ノーツタイプ (1=ドン, 2=カツ, 3=大ドン, 4=大カツ)
	double scroll = 1.0;		// ノーツのスクロール速度 (譜面読み込み時に計算して結果をここに入れる)	描画時にbpmにこれを掛けて描画
	bool hasBarline = false;	// 小節線の有無
	bool isBig = false;			// 大音符かどうか

	size_t idx;
	bool isJudged = false;  // 既に判定を下かどうかを保持
	bool isMissed = false;
	// ToDo: 連打の実装、SCROLLなどの状態の実装

	// 連打用パラメータ
	size_t pairRollIndex = SIZE_MAX;	// 連打尾のインデックスを保持
	size_t rollId;						// パース時に使用する連打のペアを保証するID

	size_t rollHitCount = 0;			// 譜面再生時に連打した回数を保持
};

struct SongData
{

	long long songStartTime;	// 曲の絶対開始時間 (us)
	long long _songProgTime;		// 曲の経過時間(再生位置) (us)
	long long offsetTime;		// 曲のオフセット時間(譜面の開始位置) (us)
	long long _songProgTimefromOffset;	// 曲のオフセットからの経過時間 (us)
	long long lastPausedTimeStamp;	// 最後に一時停止した時間 (us)
	long long totalPausedDuration; // 一時停止した時間の合計 (us)

	SoundHandle songHandle;				// 曲のハンドル
	bool playing = false;


	void playSong(bool restart = false);
	void stopSong();
	void loadSong(const char* path);
	long long getSongCurrentTimeUs(bool applyOffset = false);
	long long songProgTime() { 
		_songProgTime = GetNowHiPerformanceCount() - songStartTime - totalPausedDuration;	// GetSoundCurrentTimeによって必要なくなりました。
		return _songProgTime; 
	}
	long long songProgTimefromOffset() {
		_songProgTimefromOffset = songProgTime() - offsetTime - totalPausedDuration;
		return _songProgTimefromOffset;
	}
};


// 描画に渡す判定結果のリストの要素のデータ
struct JudgeLog {
	JudgeType type;
	long long timeStamp; // 曲時間におけるタイムスタンプ
	bool isBig = false; //大音符
};

class ChartData
{

public:
	ChartData() = default;
	ChartData(const char* path) {
		songData.loadSong(path);

	}


	SongData songData;
	std::vector<Note> notes; // マイクロ秒単位
	size_t nextNoteIndex = 0; // 判定するノーツの位置
	bool autoPlay = true; // オートプレイ

	std::string tjaPath = "";
	
	std::string Title = "";
	std::string subTitle = "";
	std::string songPath = "";
	double offset = 0.0;
	double bpm = 120.0;
	double demoStart = 0.0;

	CourseType course = CourseType::Easy;
	double level = 0.0;
	std::vector<size_t> balloon;

	long long judgeGOOD = 33000; // 良判定範囲時間(us)
	long long judgeOK = 66000; // 可判定範囲時間(us)
	long long judgeBAD = 100000; // 不可判定範囲時間(us)

	std::vector<JudgeLog> judgelogs;


	int good=0, ok=0, bad=0, miss=0; // ノーツの判定結果集計 good:良 ok:可 bad:不可 miss:叩かずにスルー
	
	int score = 0; // 現在のスコア
	

	int scoreGOOD = 200;	// 良判定のスコア ベースとなるスコア OKはGOOD/2のスコア
	int scoreOK = scoreGOOD / 2;
	int scoreBAD = 0;
	int scoreMISS = 0;
	int scoreROLL = 10;

	int combo = 0;
	int MAXcombo = 0;

	void loadSong(const char* path);
	void loadSong(const char* path, double bpm, double offset = 0.0);
	void playSong(bool restart = false);

	void init(); // 初期化処理

	void Update();
	void Input();

	void nextNotes();
	long long noteRelativeTime(size_t noteIdx);	// 音源の再生位置からのノーツの相対座標を返す

	

private: 


	const float rps = 20.0; // roll per sec
	const long long rollIntervalUs = static_cast<long long>(static_cast<float>(Time::TIME_US) / rps);
	

	long long lastRollHitUs = 0;


	void updateMissNotes();	// ノーツが通り過ぎたことを更新する
	void judgeNote();
	void autoplayHitNote();	// オートプレイの処理

	bool applyNoteJudge(Note& targetNote, JudgeType judgeType);
};

