#pragma once
// Test-only audio, clock and keyboard boundary. No renderer or real audio device.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
using std::max;
using std::abs;
constexpr int TRUE = 1;
constexpr int DX_PLAYTYPE_BACK = 1;
enum { KEY_INPUT_D=1, KEY_INPUT_F, KEY_INPUT_J, KEY_INPUT_K, KEY_INPUT_SPACE,
    KEY_INPUT_BACK, KEY_INPUT_KANJI, KEY_INPUT_F1, KEY_INPUT_ESCAPE };
int LoadSoundMem(const char*);
int StopSoundMem(int);
int DeleteSoundMem(int);
int PlaySoundMem(int,int,int);
int SetCurrentPositionSoundMem(long long,int);
int GetSoundCurrentTime(int);
long long GetNowHiPerformanceCount();
int GetHitKeyStateAll(char*);
int LoadGraph(const char*);
int GetGraphSize(int,int*,int*);
int DeleteGraph(int);
int GetWindowActiveFlag();
int GetWindowSize(int*,int*);
unsigned int GetColor(int,int,int);
int DrawBox(int,int,int,int,unsigned int,int);
int DrawString(int,int,const char*,unsigned int);
int DrawFormatString(int,int,unsigned int,const char*,...);
int SetDrawBlendMode(int,int);
void OutputDebugString(const char*);
constexpr int DX_BLENDMODE_ALPHA = 1, DX_BLENDMODE_NOBLEND = 0;
constexpr int DX_FONTTYPE_ANTIALIASING = 1;
int CreateFontToHandle(const char*,int,int,int);
int DeleteFontToHandle(int);
int GetDrawStringWidthToHandle(const char*,int,int);
int GetDrawStringWidth(const char*,int);
int DrawStringToHandle(int,int,const char*,unsigned int,int);
int DrawRoundRect(int,int,int,int,int,int,unsigned int,int);
int DrawCircle(int,int,int,unsigned int,int);
int DrawTriangle(int,int,int,int,int,int,unsigned int,int);
int DrawLine(int,int,int,int,unsigned int,int);
int DrawGraph(int,int,int,int);
