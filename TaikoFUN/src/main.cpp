#include <cstdlib>
#include <memory>
#include <vector>


#include "DxLib.h"
#include "main.h"
#include "Scenes.h"
#include "Input/Input.h"
#include "Debug/Fps.h"
#include "Core/Tween.h"
#include "Core/Time.h"
#include "Skin/SkinData.h"

GameState gGameState = GameState::Debug;
GameState preGameState = GameState::Null;



std::unique_ptr<Debug> db;

// プログラムは WinMain から始まります
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {


	SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);
	ChangeWindowMode(TRUE);
	SetGraphMode(1280, 720, 32);
	SetOutApplicationLogValidFlag(FALSE);
	SetWindowSizeExtendRate(1.0);
	SetWaitVSyncFlag(FALSE);

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}


	Skin::loadSkin();


	while (CheckHitKey(KEY_INPUT_ESCAPE) == 0 && ProcessMessage() == 0)
	{


		if (gGameState != preGameState) {
			if (gGameState == GameState::Debug) {
				db = std::make_unique<Debug>();
			}
			if (preGameState == GameState::Debug) {
				db.reset();
			}
		}


	Update(gGameState);

	Draw(gGameState);
	
	}
	
	db.reset();
	DxLib_End();				// ＤＸライブラリ使用の終了処理

	return 0;				// ソフトの終了
}

void Update(GameState state) {

	Time::Update();


	Input::Update();
	FPS::Update();

	switch (state)
	{
	case GameState::Title:

		break;
	case GameState::Playing:

		break;	
	case GameState::Result:

		break;
	case GameState::Debug:
		
		db->Update();
		break;
	default:
		break;
	}



	preGameState = gGameState;
}

void Draw(GameState state) {

	DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);


	switch (state)
	{
	case GameState::Title:

		break;
	case GameState::Playing:

		break;
	case GameState::Result:

		break;
	case GameState::Debug:

		db->Draw();
		break;
	default:
		break;
	}

	ScreenFlip();

}
