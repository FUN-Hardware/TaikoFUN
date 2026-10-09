#include "BackgroundRenderer.h"

#include "Skin/SkinData.h"
#include "Scene/Play/Logic/ChartPlayer.h"



void BackgroundRenderer::Draw(const ChartPlayer& cp_) {

	if ( cp_.isGogoTime() ) {
		DrawGraph( 0, 0, Skin::GetTexture( "play/bg_clear" ).handle, true );
	}
	else {
		DrawGraph( 0, 0, Skin::GetTexture( "play/bg" ).handle, true );
	}
	

	
}

