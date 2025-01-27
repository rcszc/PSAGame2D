// base_define_header.
#include "base_define_header.hpp"

using namespace std;
using namespace PSAG_LOGGER;

Mptr::DefinePointer<PsagManager::Notify::NotifySystem> ZPGameGlobale::GLO_Notify = {};
Mptr::DefinePointer<PsagActor::ActorsManager>* ZPGameGlobale::GLO_NpcActorsRef = nullptr;
size_t ZPGameGlobale::GLO_PlayerBullets = 0;
float ZPGameGlobale::GLO_PlayerDamageCount = 0.0f;