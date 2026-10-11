#include "Scene/SongSelect/SongSelectView.h"
#include "stubs/DrawRecorder.h"
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <limits>
namespace { int graphs=0; bool failGraph=false; }
int LoadGraph(const char*) { if (failGraph) return -1; ++graphs; return 1; }
int DeleteGraph(int) { --graphs; return 0; }
void Check(bool ok,const char* message) { if (!ok) { std::cerr<<message<<'\n'; std::exit(1); } }
int main() {
    std::filesystem::create_directories("captures");
    std::vector<SongItemData> songs(12);
    const char* titles[]={"シャイニングスター","キスキツネ","スカイタワー","おどるポンポコリン",
        "フィクサー","聖少女領域","ごはん食べヨ","続・〆ドレー2000","ドンカマ2000",
        "万戈イムー一ノ十","8OROCHI","とても長い日本語の曲名でも表示が崩れないための選曲画面テスト・特別バージョン"};
    for (size_t i=0;i<songs.size();++i) {
        songs[i].title=titles[i];
        for (size_t c=0;c<5;++c) { songs[i].Courses[c]=c<4; songs[i].levels[c]=float(2+c*2); }
    }
    songs[4].Courses[0]=false;
    {
        SongSelectView view; view.Init();
        auto capture=[&](const char* name,size_t selected,size_t course,bool choosing,bool preview,const std::string& error="",double fade=0.0) {
            view.Update(0.016,selected,choosing);
            DrawRecorder::Begin(std::string("captures/")+name+".jsonl");
            view.Draw(songs,selected,course,choosing,preview,error,fade);
            DrawRecorder::End();
            Check(DrawRecorder::BlendReset(),"view leaked alpha blend state");
        };
        capture("song-list",3,0,false,true);
        capture("difficulty",3,2,true,true);
        capture("unavailable",4,0,true,true);
        capture("long-title",11,3,false,true);
        capture("error",2,1,false,false,"この曲を開始できません");
        capture("return-fade",2,1,false,true,"",0.2);
        songs.resize(1); capture("one-song",0,0,false,false);
        songs.clear(); capture("empty",0,0,false,false);
        view.Finalize(); view.Finalize();
        Check(graphs==0 && DrawRecorder::LiveFonts()==0,"resources not released");
        failGraph=true; DrawRecorder::FailFonts(true); view.Init();
        songs.emplace_back(); songs[0].title="素材が読み込めない場合の選曲表示";
        songs[0].Courses[3]=true; songs[0].levels[3]=5;
        view.Update(std::numeric_limits<double>::quiet_NaN(),0,false);
        capture("missing-assets",0,3,true,false);
    }
    Check(graphs==0 && DrawRecorder::LiveFonts()==0,"destructor resource leak");
    std::cout<<"View captures and lifecycle checks passed\n";
}
