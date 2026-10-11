#pragma once
#include <string>
namespace DrawRecorder {
void Begin(const std::string& path);
void End();
int LiveFonts();
bool BlendReset();
void FailFonts(bool fail);
}
