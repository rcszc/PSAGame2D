// psag_physics_engine.
#include "psag_physics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

bool ONLY_ONE_FLAG_SET(uint16_t enumflags) {
	return enumflags != 0 && (enumflags & (enumflags - 1)) == 0;
}

float __PHYSICS_ENGINE_TIMESETP::PhysicsEngineTimeStep = 1.0f;
namespace PhysicsEngine {
	// 碰撞(事件)发生.
	void __PsagPhyContactListener::BeginContact(b2Contact* contact) {
		// get collision.begin fixture a,b.
		b2Fixture* FixtureA = contact->GetFixtureA();
		b2Fixture* FixtureB = contact->GetFixtureB();

		b2Body* BodyA = FixtureA->GetBody();
		b2Body* BodyB = FixtureB->GetBody();

		// processing sensor event => return.
		if (FixtureA->IsSensor() || FixtureB->IsSensor()) {
			// sensor and sensor => invalid.
			if (FixtureA->IsSensor() && FixtureB->IsSensor()) return;
			// storage collision_event.
			PhysicsSensor[PhysicsCollisionData(BodyA, BodyB)] =
				PhysicsCollisionCode(PhysicsDataset[BodyA].BindUniqueCode, PhysicsDataset[BodyB].BindUniqueCode);
			return;
		}

		// collision a,b =find=> unique_code.
		PhysicsCollision[__PsagPhyCollisionKey(BodyA, BodyB)] =
			PhysicsCollisionCode(PhysicsDataset[BodyA].BindUniqueCode, PhysicsDataset[BodyB].BindUniqueCode);
	}

	// 碰撞(事件)结束.
	void __PsagPhyContactListener::EndContact(b2Contact* contact) {
		// get collision.end fixture a,b.
		b2Fixture* FixtureA = contact->GetFixtureA();
		b2Fixture* FixtureB = contact->GetFixtureB();

		b2Body* BodyA = FixtureA->GetBody();
		b2Body* BodyB = FixtureB->GetBody();

		// processing sensor event => return.
		if (FixtureA->IsSensor() || FixtureB->IsSensor()) {
			// sensor and sensor => invalid.
			if (FixtureA->IsSensor() && FixtureB->IsSensor()) return;
			// delete coll_event.
			PhysicsSensor.erase(__PsagPhyCollisionKey(BodyA, BodyB));
			return;
		}
		// delete collision item.
		PhysicsCollision.erase(__PsagPhyCollisionKey(BodyA, BodyB));
	}

	unordered_map<string, PhysiceWorldData> PhyEngineCoreDataset::PhysicsWorlds = {};
	// global cycles calc iterations.
	Vector2T<float> PhyEngineCoreDataset::PhysicsIterations = Vector2T<float>(16.0f, 8.0f);

	vector<b2Vec2> PresetVertexGroupRECT() {
		vector<b2Vec2> CreateVertGroup = {};
		CreateVertGroup.push_back(b2Vec2(-10.0f, -10.0f));
		CreateVertGroup.push_back(b2Vec2( 10.0f, -10.0f));
		CreateVertGroup.push_back(b2Vec2( 10.0f,  10.0f));
		CreateVertGroup.push_back(b2Vec2(-10.0f,  10.0f));
		return CreateVertGroup;
	}

	vector<b2Vec2> PresetVertexGroupCIRCLE(const Vector2T<float>& scale, uint32_t num) {
		float STEP = 2.0f * PSAG_M_PI / num;

		vector<b2Vec2> CreateVertGroup = {};
		for (size_t i = 0; i < num; ++i)
			CreateVertGroup.push_back(b2Vec2(
				cos((float)i * STEP) * 10.0f * scale.vector_x, 
				sin((float)i * STEP) * 10.0f * scale.vector_y
			));
		return CreateVertGroup;
	}

	vector<b2Vec2> VertexPosToBox2dVec(const vector<Vector2T<float>>& data) {
		vector<b2Vec2> DatasetTemp = {};
		for (const auto& Vert : data)
			DatasetTemp.push_back(b2Vec2(Vert.vector_x, Vert.vector_y));
		return DatasetTemp;
	}

	bool PhyEngineCoreDataset::PhyBodyItemAlloc(string world, PhyBodyKey* rukey, PhysicsBodyConfig config) {
		auto WorldPointer = PhysicsWorldFind(world);
		if (WorldPointer == nullptr) {
			PushLogger(LogError, PSAGM_PHYENGINE_LABEL, "body_data: unable find world.");
			return false;
		}
		auto it = WorldPointer->PhysicsContact->PhysicsDataset.find(*rukey);
		if (it != WorldPointer->PhysicsContact->PhysicsDataset.end()) {
			PushLogger(LogWarning, PSAGM_PHYENGINE_LABEL, "body_data: failed alloc duplicate_key: %llu", *rukey);
			return false;
		}
		/*
		if (!ONLY_ONE_FLAG_SET(config.PhysicsCollisionThis)) {
			PushLogger(LogError, PSAGM_PHYENGINE_LABEL, "body_data: filter: this_value only one.");
			return false;
		}
		*/
		b2Filter FILTER;
		FILTER.categoryBits = config.PhysicsCollisionThis;
		FILTER.maskBits     = config.PhysicsCollisionFilter;
		FILTER.groupIndex   = 0; // default group.

		// 创建 Actor 物理碰撞.
		b2BodyDef DefineBody;
		if (config.PhysicsModeTypeFlag) DefineBody.type = b2_dynamicBody;
		else                            DefineBody.type = b2_staticBody;

		DefineBody.position = b2Vec2(config.PhyBoxPosition.vector_x, config.PhyBoxPosition.vector_y);
		DefineBody.angle    = config.PhyBoxRotate;
		// world(global) => create body item.
		b2Body* BodyData = WorldPointer->PhysicsWorld->CreateBody(&DefineBody);

		b2PolygonShape CollisionBox;
		b2CircleShape  CollisionCircle;

		b2FixtureDef DefineFixture;
		switch (config.PhysicalShapeType) {
			// 多边形碰撞箱.
		case(POLYGON_TYPE): {
			// process scale.
			for (auto& ScaleVert : config.CollVertexGroup) {
				ScaleVert.x *= config.PhyBoxCollisionSize.vector_x;
				ScaleVert.y *= config.PhyBoxCollisionSize.vector_y;
			}
			CollisionBox.Set(config.CollVertexGroup.data(), (int32)config.CollVertexGroup.size());
			// shape(idx) => fixture.
			DefineFixture.shape = &CollisionBox;
			break;
		}
		case(CIRCLE_TYPE): {
			// circle position,radius.
			CollisionCircle.m_p.Set(config.PhyBoxPosition.vector_x, config.PhyBoxPosition.vector_y);
			CollisionCircle.m_radius = config.PhyBoxCollisionSize.vector_x;
			// shape(idx) => fixture.
			DefineFixture.shape = &CollisionCircle;
			break;
		}}

		if (config.PhysicsIsSensorFlag) {
			// Body 配置为传感器(无物理碰撞).
			DefineFixture.isSensor = true;
		}
		else {
			DefineFixture.density  = config.PhyBodyDensity;
			DefineFixture.friction = config.PhyBodyFriction;
			DefineFixture.filter   = FILTER;
		}

		// 关闭 Body 碰撞.
		if (!config.PhysicsCollisionFlag)
			DefineFixture.filter.maskBits = NULL;

		BodyData->CreateFixture(&DefineFixture);
		BodyData->SetAwake (true);
		BodyData->SetBullet(true);

		if (BodyData == nullptr) {
			PushLogger(LogError, PSAGM_PHYENGINE_LABEL, "body_data: failed alloc, data = nullptr.");
			return false;
		}
		// key = body_data pointer.
		*rukey = BodyData;
		WorldPointer->PhysicsContact->PhysicsDataset[*rukey] = PhysicsBodyData(config.IndexUniqueCode, BodyData);

		PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "phy_world => body_data, name: %s", world.c_str());
		if (config.PhysicsModeTypeFlag) PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "body_data(dynamic) item: alloc key: %llu", rukey);
		else                            PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "body_data(static) item: alloc key: %llu", rukey);
		return true;
	}

	bool PhyEngineCoreDataset::PhyBodyItemFree(string world, PhyBodyKey rukey) {
		auto WorldPointer = PhysicsWorldFind(world);
		if (WorldPointer == nullptr) {
			PushLogger(LogError, PSAGM_PHYENGINE_LABEL, "body_data: unable find world.");
			return false;
		}
		auto it = WorldPointer->PhysicsContact->PhysicsDataset.find(rukey);
		if (it != WorldPointer->PhysicsContact->PhysicsDataset.end()) {
			// free ersae data.
			WorldPointer->PhysicsWorld->DestroyBody(it->second.Box2dActorPtr);
			WorldPointer->PhysicsContact->PhysicsDataset.erase(it);
			PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "phy_world => body_data, name: %s", world.c_str());
			PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "body_data item: delete key: %llu", rukey);
			return true;
		}
		PushLogger(LogWarning, PSAGM_PHYENGINE_LABEL, "body_data item: failed delete, not found key.");
		return false;
	}

	void PhyEngineCoreDataset::PhyBodyItemResetBox(string world, PhyBodyKey rukey, Vector2T<float> size, float density, float friction) {
		auto WorldPointer = PhysicsWorldFind(world);
		if (WorldPointer == nullptr) {
			PushLogger(LogError, PSAGM_PHYENGINE_LABEL, "body_data: unable find world.");
			return;
		}
		auto it = WorldPointer->PhysicsContact->PhysicsDataset.find(rukey);
		if (it != WorldPointer->PhysicsContact->PhysicsDataset.end()) {
			auto BodyTemp = it->second.Box2dActorPtr;

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

	PhysicsRunState PhyEngineCoreDataset::PhyBodyItemGet(string world, PhyBodyKey rukey) {
		auto WorldPointer = PhysicsWorldFind(world);
		if (WorldPointer == nullptr)
			return PhysicsRunState();

#if PSAG_DEBUG_MODE
		auto it = WorldPointer->PhysicsContact->PhysicsDataset.find(rukey);
		if (it == WorldPointer->PhysicsContact->PhysicsDataset.end())
			return PhysicsRunState();
#endif
		auto BodyData = WorldPointer->PhysicsContact->PhysicsDataset[rukey];
		return PhysicsRunState(
			BodyData.Box2dActorPtr->GetAngle(),
			Vector2T<float>(BodyData.Box2dActorPtr->GetPosition().x, BodyData.Box2dActorPtr->GetPosition().y),
			BodyData.Box2dActorPtr
		);
	}

	vector<size_t> PhyEngineCoreDataset::PhyBodyItemGetCollision(string world, PhyBodyKey rukey, int mode) {
		vector<size_t> TotalUnqiueIndex = {};
		b2Body* BodyCollided = nullptr;

		auto WorldPointer = PhysicsWorldFind(world);
		if (WorldPointer == nullptr)
			return TotalUnqiueIndex;

		CollisionHashMap* CollisionDataList = nullptr;
		auto it = WorldPointer->PhysicsContact->PhysicsDataset.find(rukey);

		switch (mode) {
		case(1): { CollisionDataList = &WorldPointer->PhysicsContact->PhysicsCollision; break; }
		case(2): { CollisionDataList = &WorldPointer->PhysicsContact->PhysicsSensor;    break; }
		}
#if PSAG_DEBUG_MODE
		if (CollisionDataList == nullptr)
			return TotalUnqiueIndex;
#endif
		for (const auto& COLL : WorldPointer->PhysicsContact->PhysicsCollision) {
			// mode: this_actor => actor : actor => this_actor.
			if (COLL.first.CollisionBodyA == it->second.Box2dActorPtr ||
				COLL.first.CollisionBodyB == it->second.Box2dActorPtr
			) {
				COLL.second.BindUniqueCodeA == it->second.BindUniqueCode ? 
					TotalUnqiueIndex.push_back(COLL.second.BindUniqueCodeB) : TotalUnqiueIndex.push_back(COLL.second.BindUniqueCodeA);
			}
		}
		return TotalUnqiueIndex;
	}

	size_t PhyEngineCoreDataset::PhyBodyItemGetCollisionFirst(string world, PhyBodyKey rukey, int mode) {
		size_t FirstUnqiueIndex = NULL;
		b2Body* BodyCollided = nullptr;

		auto WorldPointer = PhysicsWorldFind(world);
		if (WorldPointer == nullptr)
			return FirstUnqiueIndex;

		CollisionHashMap* CollisionDataList = nullptr;
		auto it = WorldPointer->PhysicsContact->PhysicsDataset.find(rukey);

		switch (mode) {
		case(1): { CollisionDataList = &WorldPointer->PhysicsContact->PhysicsCollision; break; }
		case(2): { CollisionDataList = &WorldPointer->PhysicsContact->PhysicsSensor;    break; }
		}
#if PSAG_DEBUG_MODE
		if (CollisionDataList == nullptr)
			return NULL;
#endif
		for (const auto& COLL : (*CollisionDataList)) {
			// mode: this_actor => actor : actor => this_actor.
			if (COLL.first.CollisionBodyA == it->second.Box2dActorPtr ||
				COLL.first.CollisionBodyB == it->second.Box2dActorPtr
			) {
				COLL.second.BindUniqueCodeA == it->second.BindUniqueCode ?
					FirstUnqiueIndex = COLL.second.BindUniqueCodeB : FirstUnqiueIndex = COLL.second.BindUniqueCodeA;
				break;
			}
		}
		return FirstUnqiueIndex;
	}

	bool PhyEngineCoreDataset::PhysicsWorldCreate(string strkey, Vector2T<float> gravity_vector) {
		if (PhysicsWorldFind(strkey) != nullptr) {
			PushLogger(LogWarning, PSAGM_PHYENGINE_LABEL, "physics_world: failed alloc duplicate_key: %s", strkey.c_str());
			return false;
		}
		PhysiceWorldData CreatePhyWorld = {};
		// create physics world.
		b2Vec2 ConfigGravity(gravity_vector.vector_x, gravity_vector.vector_y);

		CreatePhyWorld.PhysicsWorld   = new b2World(ConfigGravity);
		CreatePhyWorld.PhysicsContact = new __PsagPhyContactListener();
		// register object.
		CreatePhyWorld.PhysicsWorld->SetContactListener(CreatePhyWorld.PhysicsContact);

		PhysicsWorlds[strkey] = CreatePhyWorld;
		PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "physics_world item: alloc key: %s", strkey.c_str());
		return true;
	}

	bool PhyEngineCoreDataset::PhysicsWorldDelete(string strkey) {
		auto it = PhysicsWorlds.find(strkey);
		if (it != PhysicsWorlds.end()) {
			// free ersae data.
			size_t BodiesCount = NULL;
			for (auto& BodyItem : it->second.PhysicsContact->PhysicsDataset) {
				// free all bodies.
				it->second.PhysicsWorld->DestroyBody(BodyItem.second.Box2dActorPtr);
				++BodiesCount;
			}
			// del contact => del world => del item.
			delete it->second.PhysicsContact;
			delete it->second.PhysicsWorld;
			PhysicsWorlds.erase(it);

			PushLogger(LogInfo, PSAGM_PHYENGINE_LABEL, "physics_world item: free_bodies: %u, delete key: %s", 
				BodiesCount, strkey.c_str());
			return true;
		}
		PushLogger(LogWarning, PSAGM_PHYENGINE_LABEL, "physics_world item: failed delete, not found key.");
		return false;
	}

	PhysiceWorldData* PhyEngineCoreDataset::PhysicsWorldFind(string strkey) {
		return PhysicsWorlds.find(strkey) != PhysicsWorlds.end() ? &PhysicsWorlds[strkey] : nullptr;
	}

	void PhyEngineCoreDataset::PhysicsSystemUpdateState() {
		for (auto& WorldItem : PhysicsWorlds) {
			// box2d api update_world.
			WorldItem.second.PhysicsWorld->Step(
				PSAGM_VIR_TICKSTEP_PHY * PhysicsEngineTimeStep, (int32)PhysicsIterations.vector_x, (int32)PhysicsIterations.vector_y);
		}
	}
}