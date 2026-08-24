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

GameState gGameState = GameState::Playing;
GameState preGameState = GameState::Null;



std::unique_ptr<Debug> db;
std::unique_ptr<PlayScene> ps; // プレイシーン

// プログラムは WinMain から始まります
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {



	SetGraphMode(1280, 720, 32);
	ChangeWindowMode(TRUE);
	SetWindowSizeExtendRate(1.0);

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}

	SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);
	SetOutApplicationLogValidFlag(FALSE);
	SetWaitVSyncFlag(FALSE);
	SetDrawMode(DX_DRAWMODE_BILINEAR);


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

			if (gGameState == GameState::Playing) {
				ps = std::make_unique<PlayScene>();
			}
			if (preGameState == GameState::Playing) {
				ps.reset();
			}

		}


	Update(gGameState);

	Draw(gGameState);
	
	}
	
	db.reset();
	ps.reset();
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
		
		ps->Update();
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

		ps->Draw();
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
