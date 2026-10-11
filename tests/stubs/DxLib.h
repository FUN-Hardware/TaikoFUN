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
