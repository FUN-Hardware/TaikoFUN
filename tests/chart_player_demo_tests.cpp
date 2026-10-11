#include "Scene/Play/Logic/ChartPlayer.h"
#include "Input/Input.h"
#include "Skin/SkinData.h"
#include <array>
#include <cstring>
#include <cstdlib>
#include <iostream>

namespace {
long long counter = 1000000000;
int audioMs = 0, songStarts = 0;
std::array<char,256> keys{};
void Check(bool value, const char* message) {
    if (!value) { std::cerr << message << '\n'; std::exit(1); }
}
void At(int ms) { audioMs=ms; counter=1000000000LL+ms*1000LL; }
Note MakeNote(NoteType type, size_t index, long long time) {
    Note note; note.type=type; note.idx=index; note.absTime=time; return note;
}
void ReleaseKeys() { keys.fill(0); Input::Update(); }
}

int LoadSoundMem(const char*) { return 1; }
int StopSoundMem(int) { return 0; }
int DeleteSoundMem(int) { return 0; }
int PlaySoundMem(int handle,int,int restart) {
    if (handle==1) { ++songStarts; if (restart) audioMs=0; }
    return 0;
}
int SetCurrentPositionSoundMem(long long,int) { return 0; }
int GetSoundCurrentTime(int) { return audioMs; }
long long GetNowHiPerformanceCount() { return counter; }
int GetHitKeyStateAll(char* state) { std::memcpy(state,keys.data(),keys.size()); return 0; }
int LoadGraph(const char*) { return 1; }
int GetGraphSize(int,int* w,int* h) { *w=*h=1; return 0; }
int DeleteGraph(int) { return 0; }
namespace Time { long long nowTime() { return counter; } }
namespace Skin { SoundHandle& GetSound(const std::string&) { static SoundHandle hit; hit.handle=2; return hit; } }

int main() {
    ReleaseKeys();
    Check(!Input::isAnyKeyDown() && !Input::isAnyKeyTriggered(), "empty input");
    keys[KEY_INPUT_F]=1; Input::Update();
    Check(Input::isAnyKeyDown() && Input::isAnyKeyTriggered(), "fresh input");
    Input::Update();
    Check(Input::isAnyKeyDown() && !Input::isAnyKeyTriggered(), "held input is distinct from trigger");
    ReleaseKeys();

    ChartData chart{}; chart.songData.songHandle.handle=1;
    chart.notes={MakeNote(NoteType::Don,0,0),MakeNote(NoteType::Don,1,1000000)};
    At(0);
    ChartPlayer demo(chart,PlayMode::Demo);
    const auto generation=demo.getPlaybackGeneration();
    Check(demo.isAutoplay() && songStarts==1, "demo autoplays from one start");
    demo.toggleAutoplay();
    Check(demo.isAutoplay(), "demo autoplay cannot be toggled off");
    demo.Update();
    Check(chart.nowSongTime==0 && demo.getPlayStats().goodCount==1 && demo.getPlayStats().missCount==0,
        "first zero audio sample cannot become system uptime");
    keys[KEY_INPUT_SPACE]=keys[KEY_INPUT_BACK]=keys[KEY_INPUT_KANJI]=1; Input::Update();
    At(1000); demo.Update();
    Check(demo.getPlaybackGeneration()==generation && songStarts==1 && !demo.isStoped,
        "demo keyboard controls cannot restart or stop the player");
    Check(demo.isFinished && demo.getPlayStats().score==400 && demo.getPlayStats().maxCombo==2,
        "demo completes using actual judgment and combo code");
    At(2000); demo.Update();
    Check(demo.getPlayStats().goodCount==2 && demo.getJudgeEvents().empty(), "demo completion freezes stats and clears events");
    ReleaseKeys();

    ChartData roll{}; roll.songData.songHandle.handle=1;
    roll.notes={MakeNote(NoteType::RollHead,0,0),MakeNote(NoteType::RollTail,1,200000),MakeNote(NoteType::Don,2,300000)};
    roll.notes[0].pairRollIndex=1;
    At(0); ChartPlayer rollDemo(roll,PlayMode::Demo);
    for (int ms : {50,100,150,200,300}) { At(ms); rollDemo.Update(); }
    Check(rollDemo.isFinished && rollDemo.getPlayStats().rollHitCount==3 && rollDemo.getPlayStats().goodCount==1,
        "demo handles a roll followed by a regular note");

    ChartData balloon{}; balloon.songData.songHandle.handle=1;
    balloon.notes={MakeNote(NoteType::BalloonHead,0,0)};
    balloon.notes[0].duration=200000; balloon.notes[0].requiredHits=2;
    At(0); ChartPlayer balloonDemo(balloon,PlayMode::Demo);
    At(50); balloonDemo.Update(); At(100); balloonDemo.Update();
    Check(balloonDemo.isFinished && balloonDemo.getPlayStats().balloonHitCount==2 &&
        balloonDemo.getPlayStats().balloonClearCount==1, "demo clears balloons through actual player code");

    ChartData normal{}; normal.songData.songHandle.handle=1;
    normal.notes={MakeNote(NoteType::Don,0,0),MakeNote(NoteType::Don,1,1000000)};
    At(0); ChartPlayer player(normal);
    Check(!player.isAutoplay(), "normal session starts manual after demo");
    player.toggleAutoplay(); Check(player.isAutoplay(), "normal autoplay toggle is retained");
    const auto before=player.getPlaybackGeneration();
    keys[KEY_INPUT_SPACE]=1; Input::Update(); player.Update();
    Check(player.getPlaybackGeneration()==before+1, "normal restart is retained");
    ReleaseKeys(); keys[KEY_INPUT_BACK]=1; Input::Update(); player.Update();
    Check(player.isStoped, "normal abort is retained");
    std::cout << "Actual ChartPlayer, SongData and Input boundary checks passed\n";
}
