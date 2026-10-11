// Captures the production renderer's commands for offscreen layout inspection.
// Font measurement is approximate; these captures are not native DxLib screenshots.
#include "DxLib.h"
#include "DrawRecorder.h"
#include <fstream>
#include <map>
#include <string>
#include <cmath>
#include <stdexcept>
#include <initializer_list>

namespace {
std::ofstream output;
std::map<int,int> fonts;
int nextFont=0, alpha=255;
bool failFonts=false;
void Escape(const char* text) {
    output << '"';
    for (const unsigned char c : std::string(text)) {
        if (c=='"' || c=='\\') output << '\\' << c;
        else if (c<32) output << ' ';
        else output << c;
    }
    output << '"';
}
void Op(const char* op,std::initializer_list<int> values) {
    if (!output.is_open()) return;
    output << "[\"" << op << '"';
    for (int value : values) output << ',' << value;
    output << ',' << alpha << "]\n";
}
int Size(int font) { return font<0?16:fonts.at(font); }
int Width(const char* text,int length,int size) {
    double width=0;
    for (int i=0;i<length;++i) {
        const unsigned char c=static_cast<unsigned char>(text[i]);
        if (c<0x80) width+=size*0.56;
        else if ((c&0xC0)!=0x80) width+=size;
    }
    return int(std::ceil(width));
}
int Text(int x,int y,const char* text,unsigned color,int size) {
    if (output.is_open()) {
        output << "[\"text\"," << x << ',' << y << ',' << size << ',' << color << ',' << alpha << ',';
        Escape(text); output << "]\n";
    }
    return 0;
}
}
namespace DrawRecorder {
void Begin(const std::string& path) {
    output.open(path); if (!output) throw std::runtime_error("Cannot write capture");
}
void End() { output.close(); }
int LiveFonts() { return int(fonts.size()); }
bool BlendReset() { return alpha==255; }
void FailFonts(bool fail) { failFonts=fail; }
}
int CreateFontToHandle(const char*,int size,int,int) {
    if (failFonts) return -1;
    const int id=++nextFont; fonts[id]=size; return id;
}
int DeleteFontToHandle(int font) {
    if (fonts.erase(font)!=1) throw std::runtime_error("Invalid or double font release");
    return 0;
}
int GetDrawStringWidthToHandle(const char* s,int n,int font) { return Width(s,n,Size(font)); }
int GetDrawStringWidth(const char* s,int n) { return Width(s,n,16); }
int DrawStringToHandle(int x,int y,const char* s,unsigned c,int font) { return Text(x,y,s,c,Size(font)); }
int DrawString(int x,int y,const char* s,unsigned c) { return Text(x,y,s,c,16); }
int DrawFormatString(int,int,unsigned,const char*,...) { return 0; }
unsigned GetColor(int r,int g,int b) { return (r<<16)|(g<<8)|b; }
int SetDrawBlendMode(int mode,int value) { alpha=mode==DX_BLENDMODE_ALPHA?value:255; return 0; }
int DrawBox(int x,int y,int r,int b,unsigned c,int) { Op("rect",{x,y,r,b,int(c)}); return 0; }
int DrawRoundRect(int x,int y,int r,int b,int rx,int,unsigned c,int) {
    Op("round",{x,y,r,b,rx,int(c)}); return 0;
}
int DrawCircle(int x,int y,int radius,unsigned c,int) { Op("circle",{x,y,radius,int(c)}); return 0; }
int DrawTriangle(int x1,int y1,int x2,int y2,int x3,int y3,unsigned c,int) {
    Op("triangle",{x1,y1,x2,y2,x3,y3,int(c)}); return 0;
}
int DrawLine(int x,int y,int r,int b,unsigned c,int thickness) {
    Op("line",{x,y,r,b,int(c),thickness}); return 0;
}
int DrawGraph(int x,int y,int,int) { Op("background",{x,y}); return 0; }
