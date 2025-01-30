// base_define_header. 2025_01_17 RCSZ.
// game: [ZeroProject] base define header_file.
// @pomelo_star studio "PSAGame2D Framework"
// game version:
#define GAME_VERSION_ZPRO "1.0.0.20250127"
// developer(s): RCSZ.

#ifndef __BASE_DEFINE_HEADER_HPP
#define __BASE_DEFINE_HEADER_HPP
#include "../../PSAGameFrameworkCore/framework_header_actor.h"
#include "../../PSAGameFrameworkCore/framework_header_manager.h"

inline std::string SYSPATH_REF(const std::string& name) {
	return "PSAGameFrameworkDev/GameZeroProject/GameMaterial/" + name;
}
namespace Mptr = PsagManager::PtrMAG;

using ShaderCode = const char*;

class ZPGameGlobale {
protected:
	// scene module: create, delete ptr.
	static Mptr::DefinePointer<PsagManager::Notify::NotifySystem> GLO_Notify;
	static Mptr::DefinePointer<PsagActor::ActorsManager>* GLO_NpcActorsRef;
	static size_t GLO_PlayerBullets;
	static float GLO_PlayerDamageCount;
};

struct PPActorBulletFire {
	float Angle = 0.0f;
	Vector2T<float> Position = {};
};

struct QueryHitNpcActor {
	ResUnique UniqueCode = NULL;
	float BulletDamage = 0.0f;
};

#endif