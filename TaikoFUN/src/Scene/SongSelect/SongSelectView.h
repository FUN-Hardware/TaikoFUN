#pragma once
#include "SongItemData.h"
#include <array>
#include <string>
#include <vector>

// Presentation only: selection, playback and scene requests stay in SongSelect.
class SongSelectView {
public:
    SongSelectView() = default;
    ~SongSelectView() { Finalize(); }
    SongSelectView(const SongSelectView&) = delete;
    SongSelectView& operator=(const SongSelectView&) = delete;
    void Init();
    void Finalize();
    void Update(double delta, size_t selected, bool choosingCourse);
    void Draw(const std::vector<SongItemData>& songs, size_t selected, size_t course,
        bool choosingCourse, bool previewPlaying, const std::string& error, double returnFade) const;
private:
    enum Font { Small, Label, Body, Title, Number };
    std::array<int,5> fonts_{-1,-1,-1,-1,-1};
    int background_ = -1;
    size_t previousSong_ = size_t(-1);
    bool previousChoosing_ = false;
    double selectionAge_ = 1.0;
    double clock_ = 0.0;
    int Width(const std::string& text, Font font) const;
    void Text(int x, int y, const std::string& text, Font font, unsigned color) const;
    void Center(int x, int y, const std::string& text, Font font, unsigned color) const;
    std::string Fit(const std::string& text, Font font, int width) const;
    void TitleLines(const std::string& title) const;
};
