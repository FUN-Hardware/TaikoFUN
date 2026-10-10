#pragma once


#include "Scene.h"
#include "SceneContexts.h"

#include <memory>


// シーンを管理するクラス
// APIスタイルの実装 .cpp内に宣言・実装、生成し、.hにはゲッターやセッターのみを書く。これにより、参照される必要のないオブジェクトを隠蔽することができる。


class SceneManager
{

public:
	void Init(SceneID firstScene, GameContext* ctx) {
		// 初期化処理
		ctx_ = ctx;
		currentScene = Create(firstScene);
		currentScene->Init();
	}

	void Update() {
		// 更新処理
		if ( currentScene )
		{
			currentScene->Update();
		}


		if ( currentScene->Request() ) ChangeScene(*currentScene->Request());
	}

	void Draw() {
		// 描画処理
		currentScene->Draw();
	}

	void Finalize() {
		// 終了処理
		

	}

	void ChangeScene( SceneID newSceneID ) {
		// シーンの切り替え処理

		if ( currentScene ) currentScene->Finalize();

		currentScene = Create(newSceneID);

		currentScene->Init();

	}

private:

	std::unique_ptr<Scene> currentScene = nullptr; // 現在のシーンを保持するポインタ
	GameContext* ctx_; // ゲームの状態を保持するコンテキスト


	std::unique_ptr<Scene> Create( SceneID id );
};