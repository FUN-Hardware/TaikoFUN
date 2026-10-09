#include "SceneManager.h"

#include "Scene.h"
#include "SceneID.h"
#include "SceneContexts.h"
#include "Play/Play.h"

#include <memory>



std::unique_ptr<Scene> SceneManager::Create( SceneID id ) {
	switch ( id ) {
		case SceneID::Play: return std::make_unique<PlayScene>( ctx_ );   // ctxを渡すだけ
		case SceneID::SongSelect: return std::make_unique<PlayScene>( ctx_);
	}

}

