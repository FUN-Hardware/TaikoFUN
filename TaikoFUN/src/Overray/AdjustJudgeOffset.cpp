#include "AdjustJudgeOffset.h"
#include "Dxlib.h"
#include "Input/Input.h"
#include "Scene/SceneContexts.h"
#include "Core/Time.h"
#include "Core/text.h"

#include <string>
void AdjustJudgeOffset::Update() {

	if (offsetShowTimer_ > 0) offsetShowTimer_ -= Time::deltaSec();
	Input();

}

void AdjustJudgeOffset::Input() {

	if ( Input::isKeyTriggered( KEY_INPUT_RIGHT ) ) {
		ctx_->judgeOffsetUs += deltaUs;
		offsetShowTimer_ = 2.0f;
	}

	if ( Input::isKeyTriggered( KEY_INPUT_LEFT ) ) {
		ctx_->judgeOffsetUs -= deltaUs;
		offsetShowTimer_ = 2.0f;
	}

}

void AdjustJudgeOffset::Draw() {




	if (offsetShowTimer_ > 0)
	{
		int w, h;
		std::string str = "判定オフセット: " + std::to_string( ctx_->judgeOffsetUs / 100 ) + "ms";
		GetWindowSize( &w, &h );
		DrawFormatString2Center( w / 2,
								 h * (3.0f / 5.0f),
								 GetColor( 0, 0, 0 ),
								 str );
	}

}