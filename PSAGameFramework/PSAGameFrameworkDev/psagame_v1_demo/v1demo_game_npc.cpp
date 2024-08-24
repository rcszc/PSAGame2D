// v1demo_game_npc.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

class ActorLogicNPC :public PsagActor::ActorLogicBase {
public:
	void ActionLogicRun(PsagActor::Actor* actor_object, float timestep) override {

	}
};

void PsaGameV1Demo::GameCreateNPC(const Vector2T<float>& position) {
	PsagActor::ActorDESC ConfigNpcActor;
	PsagActor::ActorHpDESC PawnActorHealthDESC;

	// 创建NPC逻辑, 指针由逻辑执行组件自动销毁.
	ActorLogicNPC* NPCLogicObject = new ActorLogicNPC();

	PawnActorHealthDESC.InitialActorHealth.push_back(PsagActor::ActorHP(NpcActorHPmax, 0.1f));

	ConfigNpcActor.ActorPhysicsWorld   = "DemoPhysics";
	ConfigNpcActor.ActorShaderResource = DemoShaders->FindActorShader("PawnActor");
	ConfigNpcActor.ActorHealthSystem   = PawnActorHealthDESC;
	ConfigNpcActor.ActorLogicObject    = NPCLogicObject;

	ConfigNpcActor.ForceClacEnable = false;
	ConfigNpcActor.EnableLogic     = true;

	ConfigNpcActor.InitialPhysics  = Vector2T<float>(5.0f, 3.2f);
	ConfigNpcActor.InitialPosition = position;

	ConfigNpcActor.CollisionCallbackFunc = ACTOR_COLL_FUNC{
		// 与子弹发生碰撞.
		if (DemoActors->FindGameActor(
			ThisActor->ActorGetPrivate().ActorUniqueCode
		)->ActorGetPrivate().ActorTypeCode == PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorBullet")) {
			// NPC扣血 + Clamp.
			float SetHP = ThisActor->ActorGetHealth(0) <= 0.0f ?
				0.0f :
				ThisActor->ActorGetHealth(0) - PsagManager::Tools::RAND::GenerateRandomFunc(220.0f, 240.0f);
			ThisActor->ActorModifyHealth(0, SetHP);
		}
	};

	DemoActors->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorNpc"), ConfigNpcActor);
}