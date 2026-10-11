// Exercise the production SongSelect, Input and SongData across scene lifetimes.
// Only device I/O and chart discovery/loading are replaced at the boundary.
#include "Scene/SongSelect/SongSelect.h"
#include "Input/Input.h"
#include "Skin/SkinData.h"
#include "File/ChartLoader.h"
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>

namespace {
std::array<char,256> keys{};
double dt = 1.0 / 60.0;
bool focused = true;
std::vector<std::string> paths{"Songs/first.tja", "Songs/second.tja"};
std::string audioPath;
int activePreview = 0, nextHandle = 0;
void Check(bool ok, const char* message) {
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
void Frame(SongSelect& scene, int key = 0) {
    keys.fill(0);
    if (key) keys[key] = 1;
    Input::Update(); scene.Update(); scene.Draw();
}
void Tap(SongSelect& scene, int key) { Frame(scene); Frame(scene,key); }
}
int LoadSoundMem(const char*) { return ++nextHandle; }
int StopSoundMem(int handle) { if (activePreview == handle) activePreview = 0; return 0; }
int DeleteSoundMem(int handle) { return StopSoundMem(handle); }
int PlaySoundMem(int handle,int,int) { if (handle > 0) activePreview = handle; return 0; }
int SetCurrentPositionSoundMem(long long,int) { return 0; }
int GetSoundCurrentTime(int) { return 0; }
long long GetNowHiPerformanceCount() { return 1000000000; }
int GetHitKeyStateAll(char* state) { std::memcpy(state,keys.data(),keys.size()); return 0; }
int LoadGraph(const char*) { return 1; }
int GetGraphSize(int,int* w,int* h) { *w=*h=1; return 0; }
int DeleteGraph(int) { return 0; }
int GetWindowActiveFlag() { return focused; }
int GetWindowSize(int* w,int* h) { *w=1280; *h=720; return 0; }
unsigned int GetColor(int,int,int) { return 0; }
int DrawBox(int,int,int,int,unsigned int,int) { return 0; }
int DrawString(int,int,const char*,unsigned int) { return 0; }
int DrawFormatString(int,int,unsigned int,const char*,...) { return 0; }
int SetDrawBlendMode(int,int) { return 0; }
void OutputDebugString(const char*) {}
void DrawFormatString2Right(int,int,unsigned int,std::string) {}
namespace Time {
long long nowTime() { return GetNowHiPerformanceCount(); }
double deltaSec() { return dt; }
double deltaUs() { return dt * 1000000; }
}
namespace Skin {
SoundHandle& GetSound(const std::string&) { static SoundHandle hit; return hit; }
}
std::vector<std::string> FindAllTjaFiles(const std::string&) { return paths; }
namespace file_util {
std::vector<std::string> getAllLines(const std::string& path) {
    return {"TITLE:" + path, "WAVE:" + audioPath, "COURSE:Oni", "LEVEL:3"};
}
}
namespace ChartLoad {
std::shared_ptr<ChartData> load(const char*, CourseType) {
    auto chart = std::make_shared<ChartData>();
    chart->songData.loadSong(audioPath.c_str());
    chart->notes.emplace_back();
    return chart;
}
}

int main() {
    const auto fixture = std::filesystem::temp_directory_path() / "taikofun-song-select-test.wav";
    audioPath = fixture.string();
    std::ofstream(fixture).put('\0');
    GameContext ctx;
    {
        SongSelect menu(&ctx); menu.Init(); Frame(menu);
        Check(ctx.songSelectCursor.path == paths[0], "first Update saves a valid cursor");
        for (int i=0; i<100; ++i) {
            Tap(menu,KEY_INPUT_D);
            Check(ctx.songSelectCursor.path == paths[1], "left wraps to last song");
            Tap(menu,KEY_INPUT_K);
            Check(ctx.songSelectCursor.path == paths[0], "right wraps to first song");
        }
        Tap(menu,KEY_INPUT_K);
        dt=1.0;
        for (int i=0; i<31; ++i) Frame(menu);
        Check(menu.Request() == SceneID::Play && ctx.playMode == PlayMode::Demo,
            "idle starts demo through actual menu");
        Check(ctx.selectedCourse.path == paths[1] && ctx.selectedCourse.course == CourseType::Oni,
            "demo selects available course");
        Check(ctx.songSelectCursor.course == CourseType::Easy, "fallback does not change cursor difficulty");
        menu.Finalize();
        Check(activePreview == 0, "preview stopped before leaving menu");
    }
    ctx.chartData.reset(); ctx.blockSongSelectInputUntilRelease=true;
    dt=1.0/60.0;
    {
        SongSelect menu(&ctx); menu.Init();
        Frame(menu,KEY_INPUT_F); Frame(menu,KEY_INPUT_F); Frame(menu);
        Check(!menu.Request() && ctx.playMode == PlayMode::Normal, "demo return gates held input");
        Frame(menu);
        Check(ctx.songSelectCursor.path == paths[1], "new scene restores song cursor");
        Tap(menu,KEY_INPUT_F); // course selection
        Tap(menu,KEY_INPUT_K); Tap(menu,KEY_INPUT_K); Tap(menu,KEY_INPUT_K);
        Tap(menu,KEY_INPUT_F);
        Check(menu.Request() == SceneID::Play && ctx.playMode == PlayMode::Normal && ctx.chartData,
            "normal play still starts after demo return");
        menu.Finalize();
    }
    ctx.chartData.reset();
    paths.clear();
    {
        SongSelect menu(&ctx); menu.Init();
        for (int key : {0,KEY_INPUT_D,KEY_INPUT_K,KEY_INPUT_F}) Frame(menu,key);
        menu.Finalize();
        Check(!menu.Request(), "empty catalog remains safe");
    }
    paths={"Songs/replacement.tja"};
    {
        SongSelect menu(&ctx); menu.Init();
        Frame(menu); Tap(menu,KEY_INPUT_D); Tap(menu,KEY_INPUT_K);
        Check(ctx.songSelectCursor.path == paths[0], "removed saved song falls back safely");
        menu.Finalize();
    }
    std::filesystem::remove(fixture);
    std::cout << "Actual SongSelect scene lifecycle checks passed\n";
}
