// psag_physics_engine.
#include "psag_physics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PhysicsEngine {
	unordered_map<string, PhysicsBodyData> PhyEngineObjectData::PhysicsDataset = {};

	b2World* PhyEngineObjectData::PhysicsWorld       = nullptr;
	b2Body*  PhyEngineObjectData::PhysicsWorldGround = nullptr;

	Vector2T<float> PhyEngineObjectData::PhysicsIterations = Vector2T<float>(16.0f, 8.0f);

	bool PhyEngineObjectData::PhyBodyItemAlloc(ResUnique strkey, PhysicsBodyConfig config) {
		auto it = PhysicsDataset.find(strkey);
		if (it != PhysicsDataset.end()) {
			PushLogger(LogWarning, PSAGM_PHYENGINE_LABEL, "body_data: failed alloc duplicate_key: %s", strkey.c_str());
			return false;
		}
		// 创建 Actor 物理碰撞.
		b2BodyDef DefineBody;
		if (config.PhysicsModeTypeFlag) DefineBody.type = b2_dynamicBody;
		else                            DefineBody.type = b2_staticBody;

		DefineBody.position = b2Vec2(config.PhyBoxPosition.vector_x, config.PhyBoxPosition.vector_y);
		DefineBody.angle    = config.PhyBoxRotate;
		// world(global) => create body item.
		b2Body* BodyData = PhysicsWorld->CreateBody(&DefineBody);

		b2PolygonShape CollisionBox;
		CollisionBox.SetAsBox(config.PhyBoxCollisionSize.vector_x, config.PhyBoxCollisionSize.vector_y);

		b2FixtureDef DefineFixture;
		DefineFixture.shape    = &CollisionBox;
		DefineFixture.density  = config.PhyBodyDensity;
		DefineFixture.friction = config.PhyBodyFriction;
		BodyData->CreateFixture(&DefineFixture);
		BodyData->SetBullet(true);

		if (BodyData == nullptr) {
			PushLogger(LogError, PSAGM_PHYENGINE_LABEL, "body_data: failed alloc, data = nullptr.");
			return false;
		}
		PhysicsDataset[strkey] = PhysicsBodyData(config.IndexUniqueCode, PhysicsWorldGround, BodyData);

		if (config.PhysicsModeTypeFlag) PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "body_data(dynamic) item: alloc key: %s", strkey.c_str());
		else                            PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "body_data(static) item: alloc key: %s", strkey.c_str());
		return true;
	}

	bool PhyEngineObjectData::PhyBodyItemFree(ResUnique strkey) {
		auto it = PhysicsDataset.find(strkey);
		if (it != PhysicsDataset.end()) {
			// free ersae data.
			PhysicsWorld->DestroyBody(it->second.Box2dActorPointer);
			PhysicsDataset.erase(it);
			PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "body_data item: delete key: %s", strkey.c_str());
			return true;
		}
		PushLogger(LogWarning, PSAGM_PHYENGINE_LABEL, "body_data item: failed delete, not found key.");
		return false;
	}

	void PhyEngineObjectData::PhyBodyItemResetBox(ResUnique strkey, Vector2T<float> size, float density, float friction) {
		auto it = PhysicsDataset.find(strkey);
		if (it != PhysicsDataset.end()) {
			auto BodyTemp = it->second.Box2dActorPointer;

			b2Fixture* Fixture = BodyTemp->GetFixtureList();
			while (Fixture) {
				b2Fixture* NextTmp = Fixture->GetNext();
				BodyTemp->DestroyFixture(Fixture);
				Fixture = NextTmp;
			}

			b2PolygonShape NewShape;
			NewShape.SetAsBox(size.vector_x, size.vector_y);

			b2FixtureDef FixtureDefine;
			FixtureDefine.shape    = &NewShape;
			FixtureDefine.density  = density;
			FixtureDefine.friction = friction;
			BodyTemp->CreateFixture(&FixtureDefine);
		}
	}

	PhysicsRunState PhyEngineObjectData::PhyBodyItemGet(ResUnique strkey) {
		auto it = PhysicsDataset.find(strkey);
		if (it != PhysicsDataset.end()) {
			return PhysicsRunState(
				it->second.Box2dActorPointer->GetAngle(),
				Vector2T<float>(it->second.Box2dActorPointer->GetPosition().x, it->second.Box2dActorPointer->GetPosition().y),
				it->second.Box2dActorPointer
			);
		}
		return PhysicsRunState();
	}

	size_t PhyEngineObjectData::PhyBodyItemGetCollision(ResUnique strkey) {
		size_t ReturnUnqiueIndex = NULL;
		b2Body* BodyCollided = nullptr;

		auto it = PhysicsDataset.find(strkey);
		if (it != PhysicsDataset.end()) {
			auto ThisBody = it->second.Box2dActorPointer;

			for (b2Contact* Contact = PhysicsWorld->GetContactList(); Contact; Contact = Contact->GetNext()) {
				if (Contact->IsTouching() && (Contact->GetFixtureA()->GetBody() == ThisBody || Contact->GetFixtureB()->GetBody() == ThisBody)) {
					BodyCollided = Contact->GetFixtureA()->GetBody() == 
						ThisBody ? Contact->GetFixtureB()->GetBody() : Contact->GetFixtureA()->GetBody();
					break;
				}
			}
			for (const auto PhyItem : PhysicsDataset)
				if (PhyItem.second.Box2dActorPointer == BodyCollided)
					ReturnUnqiueIndex = PhyItem.second.FindUniqueCode;
		}
		return ReturnUnqiueIndex;
	}

	void PhyEngineObjectData::PhysicsObjectCreate(float width_scale) {
		// create physics world.
		b2Vec2 ConfigGravity(0.0f, 0.0f);
		PhysicsWorld = new b2World(ConfigGravity);

		PushLogger(LogTrace, PSAGM_PHYENGINE_LABEL, "physics system create.");
	}

	void PhyEngineObjectData::PhysicsObjectDelete() {
		if (PhysicsWorld == nullptr) {
			PushLogger(LogError, PSAGM_PHYENGINE_LABEL, "physics system free, world_ptr = nullptr.");
			return;
		}
		delete PhysicsWorld;
		PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "physics system free.");
	}

	void PhyEngineObjectData::PhysicsSystemUpdateState(float speed_scale) {
		// box2d api update_world.
		PhysicsWorld->Step(PSAGM_VIR_TICKSTEP_PHY * speed_scale, (int32)PhysicsIterations.vector_x, (int32)PhysicsIterations.vector_y);
	}
}