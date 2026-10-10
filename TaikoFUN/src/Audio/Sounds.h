#pragma once

// SkinDataからサウンドハンドルを取得する関数を提供するヘッダーファイル
#include "Skin/SkinData.h"

enum class SoundKey
{
	Don,
	Katsu,
	BalloonBreak,
};
namespace Sounds {

	inline int GetSoundHandle( SoundKey key ) {
		switch ( key ) {
			case SoundKey::Don:
				return Skin::GetSound( "sound/Don" ).handle;
			case SoundKey::Katsu:
				return Skin::GetSound( "sound/Katsu" ).handle;
			case SoundKey::BalloonBreak:
				return Skin::GetSound( "sound/BalloonBreak" ).handle;
			default:
				return -1; // 無効なキーの場合は-1を返す
		}
	}

}