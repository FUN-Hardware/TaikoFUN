#include "ChartDrawer.h"

#include "Dxlib.h"
#include "Scene/Play/Logic/ChartPlayer.h"
#include "SkinLayout.h"
#include "Skin/SkinData.h"


void ChartDrawer::Draw(const ChartPlayer& cp_) {

	bg_->Draw(cp_);

	// レーン
	DrawGraph( SkinLayout::ScrollField.x, SkinLayout::ScrollField.y, Skin::GetTexture( "play/ScrollField/bg" ).handle, true );

	DrawExtendGraph( SkinLayout::ScrollField.x,
					 SkinLayout::ScrollField.y,
					 SkinLayout::ScrollField.x + SkinLayout::NoteSize,
					 SkinLayout::ScrollField.y + SkinLayout::NoteSize,
					 Skin::GetTexture( Skin::GetNoteImageKey( NoteType::Judge, false ) ).handle,
					 true );

	nr_->Draw();


}

void ChartDrawer::Update(const ChartPlayer& cp_) {

}