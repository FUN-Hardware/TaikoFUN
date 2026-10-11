#include "SongSelectView.h"
#include "DxLib.h"
#include "Scene/Demo/DemoLogic.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
constexpr unsigned Ink = 0x352C32, Muted = 0x786F74, Red = 0xDA2438;
constexpr unsigned Cream = 0xFFFCF3, Line = 0xE9DFD4;
constexpr unsigned CourseColors[] = {0xBA621A,0x34815C,0x226C97,0xB63560,0x7250A1};
constexpr const char* CourseNames[] = {"かんたん","ふつう","むずかしい","おに","おに・裏"};
void Panel(int x1,int y1,int x2,int y2,unsigned color,int radius=14) {
    DrawRoundRect(x1,y1,x2,y2,radius,radius,color,TRUE);
}
// Pop one complete UTF-8 code point, including its continuation bytes.
void PopGlyph(std::string& text) {
    if (text.empty()) return;
    size_t pos=text.size()-1;
    while (pos>0 && (static_cast<unsigned char>(text[pos]) & 0xC0)==0x80) --pos;
    text.resize(pos);
}
void Drum(int x,int y,int radius) {
    DrawCircle(x+4,y+5,radius,Ink,TRUE);
    DrawCircle(x,y,radius,Cream,TRUE);
    DrawCircle(x,y,radius-5,Red,TRUE);
    DrawCircle(x-8,y-3,3,Ink,TRUE);
    DrawCircle(x+8,y-3,3,Ink,TRUE);
    DrawLine(x-8,y+9,x+8,y+9,Ink,3);
}
}

void SongSelectView::Init() {
    Finalize();
    constexpr int sizes[] = {14,18,23,36,30};
    for (size_t i=0;i<fonts_.size();++i)
        fonts_[i]=CreateFontToHandle("メイリオ",sizes[i],i>=Body?7:5,DX_FONTTYPE_ANTIALIASING);
    background_=LoadGraph("Resource/Image/SkinColor/songselectbg.png");
    previousSong_=size_t(-1); previousChoosing_=false;
    selectionAge_=1.0; clock_=0.0;
}
void SongSelectView::Finalize() {
    for (auto& font : fonts_) { if (font>=0) DeleteFontToHandle(font); font=-1; }
    if (background_>=0) DeleteGraph(background_);
    background_=-1;
}
void SongSelectView::Update(double delta,size_t selected,bool choosingCourse) {
    const double dt=std::isfinite(delta)?std::clamp(delta,0.0,0.1):0.0;
    if (previousSong_!=selected || previousChoosing_!=choosingCourse) selectionAge_=0.0;
    previousSong_=selected; previousChoosing_=choosingCourse;
    selectionAge_=(std::min)(1.0,selectionAge_+dt);
    clock_=std::fmod(clock_+dt,60.0);
}
int SongSelectView::Width(const std::string& text,Font font) const {
    return fonts_[font]>=0?GetDrawStringWidthToHandle(text.c_str(),int(text.size()),fonts_[font])
        :GetDrawStringWidth(text.c_str(),int(text.size()));
}
void SongSelectView::Text(int x,int y,const std::string& text,Font font,unsigned color) const {
    if (fonts_[font]>=0) DrawStringToHandle(x,y,text.c_str(),color,fonts_[font]);
    else DrawString(x,y,text.c_str(),color);
}
void SongSelectView::Center(int x,int y,const std::string& text,Font font,unsigned color) const {
    Text(x-Width(text,font)/2,y,text,font,color);
}
std::string SongSelectView::Fit(const std::string& text,Font font,int width) const {
    if (Width(text,font)<=width) return text;
    std::string result=text;
    while (!result.empty() && Width(result+"…",font)>width) PopGlyph(result);
    return result+"…";
}
void SongSelectView::TitleLines(const std::string& raw) const {
    const auto title=raw.empty()?std::string("タイトル未設定"):raw;
    if (Width(title,Title)<=478) { Text(70,233,title,Title,Ink); return; }
    std::string first=title;
    while (!first.empty() && Width(first,Title)>478) PopGlyph(first);
    Text(70,218,first,Title,Ink);
    Text(70,266,Fit(title.substr(first.size()),Title,478),Title,Ink);
}
void SongSelectView::Draw(const std::vector<SongItemData>& songs,size_t selected,size_t course,
    bool choosingCourse,bool previewPlaying,const std::string& error,double returnFade) const {
    // The game renders to a fixed 1280 x 720 back buffer, including fullscreen.
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND,0);
    DrawBox(0,0,1280,720,0xF6E9DC,TRUE);
    if (background_>=0) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA,70);
        DrawGraph(0,0,background_,TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND,0);
    }
    DrawBox(0,0,1280,8,Red,TRUE);
    Text(40,27,"TAIKO / FUN",Label,Red);
    Text(38,53,choosingCourse?"難易度をえらぶ":"曲をえらぶ",Title,Ink);
    Panel(404,48,520,89,choosingCourse?Line:Red,20);
    Center(462,58,"01  選曲",Label,choosingCourse?Muted:Cream);
    DrawLine(530,69,552,69,Muted,2);
    Panel(562,48,708,89,choosingCourse?Red:Line,20);
    Center(635,58,"02  難易度",Label,choosingCourse?Cream:Muted);
    Text(1020,30,"SONG COLLECTION",Small,Muted);
    char count[64];
    std::snprintf(count,sizeof(count),"%02zu / %02zu",songs.empty()?0:selected+1,songs.size());
    Text(1050,53,count,Number,Ink);
    DrawLine(40,116,1240,116,0xD7C3BC,1);

    if (songs.empty() || selected>=songs.size()) {
        Panel(180,195,1100,569,Cream,24);
        Drum(640,271,38);
        Center(640,343,"演奏できる曲がありません",Title,Ink);
        Center(640,408,"曲を追加して、もう一度ゲームを起動してください",Body,Muted);
    } else {
        const auto& song=songs[selected];
        const size_t safeCourse=(std::min)(course,size_t(4));
        Panel(44,150,614,614,0xD7C3BC,24);
        Panel(40,144,610,608,Cream,24);
        const int badgeWidth=Width("SELECTED TRACK",Small)+32;
        Panel(70,168,70+badgeWidth,195,Ink,12);
        Center(70+badgeWidth/2,173,"SELECTED TRACK",Small,Cream);
        Drum(553,180,25);
        TitleLines(song.title);
        // A restrained equalizer indicates preview playback; no extra audio owner.
        for (int i=0;i<5;++i) {
            const int height=previewPlaying?5+int(10*(0.5+0.5*std::sin(clock_*5+i))):4;
            DrawBox(72+i*6,336-height,75+i*6,336,previewPlaying?Red:Muted,TRUE);
        }
        Text(112,318,previewPlaying?"試聴中":"試聴待機中",Small,Muted);
        DrawLine(70,356,580,356,Line,1);
        Text(70,371,"難易度",Label,Ink);
        Text(400,374,choosingCourse?"カッで移動 / ドンで決定":"ドンで難易度を選択",Small,Muted);
        for (size_t i=0;i<5;++i) {
            const bool available=song.Courses[i];
            const bool focus=choosingCourse && safeCourse==i;
            const int x=70+int(i)*104;
            const int lift=focus?int(3*(1.0-std::clamp(selectionAge_/0.18,0.0,1.0))):0;
            const int y=410-lift;
            if (focus) {
                Panel(x-3,y-3,x+97,y+112,Ink,14);
                DrawTriangle(x+39,y-10,x+55,y-10,x+47,y-3,Ink,TRUE);
            }
            Panel(x,y,x+94,y+109,available?CourseColors[i]:Line,11);
            Center(x+47,y+14,CourseNames[i],Small,available?Cream:Muted);
            if (available) {
                char level[24];
                if (std::isfinite(song.levels[i]) && song.levels[i]>0)
                    std::snprintf(level,sizeof(level),"%.0f",song.levels[i]);
                else std::snprintf(level,sizeof(level),"--");
                Center(x+47,y+40,Fit(level,Number,78),Number,Cream);
                Center(x+47,y+81,"LEVEL",Small,Cream);
            } else {
                Center(x+47,y+43,"—",Number,Muted);
                Center(x+47,y+81,"譜面なし",Small,Muted);
            }
        }
        const bool unavailable=choosingCourse && !song.Courses[safeCourse];
        Panel(70,546,580,586,unavailable?Line:Red,18);
        Center(325,555,!choosingCourse?"ドン！  難易度をえらぶ":unavailable?
            "この難易度には譜面がありません":"ドン！  演奏スタート",Label,unavailable?Muted:Cream);

        Text(656,154,"曲一覧",Label,Ink);
        Text(1018,157,"カッ  D / K  で移動",Small,Muted);
        constexpr size_t Visible=7;
        const size_t start=songs.size()<=Visible?0:(std::min)(selected>3?selected-3:0,songs.size()-Visible);
        const size_t end=(std::min)(songs.size(),start+Visible);
        for (size_t i=start;i<end;++i) {
            const bool active=i==selected;
            const int y=198+int(i-start)*58;
            const int x=active?648:656;
            Panel(x,y,1206,y+51,active?Red:Cream,12);
            char number[24]; std::snprintf(number,sizeof(number),"%02zu",i+1);
            Text(x+15,y+15,number,Label,active?Cream:Muted);
            Text(x+60,y+11,Fit(songs[i].title.empty()?"タイトル未設定":songs[i].title,Body,374),Body,active?Cream:Ink);
            for (size_t c=0;c<5;++c) {
                const int cx=1116+int(c)*16;
                DrawCircle(cx,y+25,5,songs[i].Courses[c]?(active?Cream:CourseColors[c]):(active?0xB51B30:Line),TRUE);
            }
        }
        if (songs.size()>Visible) {
            Panel(1227,198,1233,597,Line,3);
            const int thumb=(std::max)(24,int(399.0*Visible/songs.size()));
            const int top=198+int((399-thumb)*double(start)/double(songs.size()-Visible));
            Panel(1227,top,1233,top+thumb,Red,3);
        }
        Text(656,615,choosingCourse?"曲を変更するには、難易度の左端で D":"好きな曲を選んで、ドンで次へ",Small,Muted);
    }
    if (!error.empty()) {
        Panel(40,614,610,643,Ink,10);
        Text(56,619,Fit(error,Small,537),Small,Cream);
    }
    DrawBox(0,657,1280,720,Ink,TRUE);
    if (songs.empty() || selected>=songs.size()) {
        Text(40,680,"曲を追加すると、ここから演奏を選べます",Label,Cream);
    } else {
    Panel(40,673,123,706,0x0086AE,8);
    Center(81,679,"D / K",Label,Cream);
    Text(137,680,choosingCourse?"難易度を移動":"曲を移動",Label,Cream);
    Panel(320,673,403,706,Red,8);
    Center(361,679,"F / J",Label,Cream);
    Text(417,680,"決定",Label,Cream);
    Text(596,683,choosingCourse?"左端で D：曲一覧へ戻る":"30秒操作がないと自動デモを開始",Small,0xE1D4CE);
    }
    Text(1115,681,"ESC  終了",Label,Cream);
    const double fade=std::isfinite(returnFade)?std::clamp(returnFade/DemoLogic::FadeSeconds,0.0,1.0):0.0;
    if (fade>0) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA,int(255*fade));
        DrawBox(0,0,1280,720,0,TRUE);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND,0);
}
