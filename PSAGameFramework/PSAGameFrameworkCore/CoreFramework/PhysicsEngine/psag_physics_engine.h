// psag_physics_engine. RCSZ. [middle_level_engine]

#ifndef __PSAG_PHYSICS_ENGINE_H
#define __PSAG_PHYSICS_ENGINE_H
#include "box2d/box2d.h"

#define ENABLE_LOWMODULE_GRAPHICS
#include "../psag_lowlevel_support.h"

#define PSAGM_VIR_TICKSTEP_PHY 0.042f

// 物理引擎"TimeStep"由"PhysicsEngine::PhyEngineCoreDataset"自身更新.
// PhysicsEngine::PhyEngineCoreDataset::PhysicsSystemUpdateState => __PHYSICS_ENGINE_TIMESETP.
class __PHYSICS_ENGINE_TIMESETP {
protected:
	static float PhysicsEngineTimeStep;
};

using PhyBodyKey = b2Body*;
// physics engine: box2d. [20240518]
namespace PhysicsEngine {
	StaticStrLABEL PSAGM_PHYENGINE_LABEL = "PSAG_PHYSICS";

	struct PhysicsBodyData {
		// bind actor_unique_code.
		size_t  BindUniqueCode;
		b2Body* Box2dActorPtr;

		PhysicsBodyData() : BindUniqueCode(NULL), Box2dActorPtr(nullptr) {}
		PhysicsBodyData(size_t code, b2Body* actor) : BindUniqueCode(code), Box2dActorPtr(actor) {}
	};

	// square, length: +-10.0f.
	std::vector<b2Vec2> PresetVertexGroupRECT();
	std::vector<b2Vec2> PresetVertexGroupCIRCLE(const Vector2T<float>& scale, uint32_t num);
	std::vector<b2Vec2> VertexPosToBox2dVec(const std::vector<Vector2T<float>>& data);

	enum BodyShapeType {
		POLYGON_TYPE = 1 << 1,
		CIRCLE_TYPE  = 1 << 2
	};
	struct PhysicsBodyConfig {
		BodyShapeType PhysicalShapeType;
		size_t IndexUniqueCode;

		// 2D碰撞顶点组 (封闭多边形).
		std::vector<b2Vec2> CollVertexGroup;

		Vector2T<float> PhyBoxPosition;
		// rect: x: width, y: height, circle: x = r (y - invalid).
		Vector2T<float> PhyBoxCollisionSize;
		float           PhyBoxRotate;

		float PhyBodyDensity;
		float PhyBodyFriction;

		// true:dynamic_body, false: static_body.
		bool PhysicsModeTypeFlag;

		uint16_t PhysicsCollisionThis;
		uint16_t PhysicsCollisionFilter;
		bool     PhysicsCollisionFlag;

		// 当Body为探测器时无碰撞体积, 碰撞信息在探测器表.
		bool PhysicsIsSensorFlag;
	};

	struct PhysicsRunState {
		float           BodyRotateAngle;
		Vector2T<float> BodyPositionMove;

		b2Body* BodySourcePointer;

		PhysicsRunState() : BodyRotateAngle(0.0f), BodyPositionMove({}), BodySourcePointer(nullptr) {}
		PhysicsRunState(float angle, const Vector2T<float>& pos, b2Body* ptr) :
			BodyRotateAngle(angle), BodyPositionMove(pos), BodySourcePointer(ptr)
		{}
	};

	// collision pairs(A,B) unique_key.
	struct PhysicsCollisionData {
		b2Body* CollisionBodyA = nullptr;
		b2Body* CollisionBodyB = nullptr;

		PhysicsCollisionData(b2Body* CA, b2Body* CB) : CollisionBodyA(CA), CollisionBodyB(CB) {}

		bool operator==(const PhysicsCollisionData& other) const {
			return (CollisionBodyA == other.CollisionBodyA) && (CollisionBodyB == other.CollisionBodyB) || 
				(CollisionBodyA == other.CollisionBodyB && CollisionBodyB == other.CollisionBodyA);
		}
	};

	struct PhysicsCollisionCode {
		// bind actor_unique_code, [a,b].
		size_t BindUniqueCodeA;
		size_t BindUniqueCodeB;

		PhysicsCollisionCode() : BindUniqueCodeA(NULL), BindUniqueCodeB(NULL) {}
		PhysicsCollisionCode(size_t CA, size_t CB) : BindUniqueCodeA(CA), BindUniqueCodeB(CB) {}
	};

	using __PsagPhyCollisionKey = PhysicsCollisionData;
	// unique_key hash.
	struct __PsagPhyCollisionKeyHASH {
		size_t operator()(const __PsagPhyCollisionKey& key) const {
			size_t HASH1 = std::hash<b2Body*>()(key.CollisionBodyA);
			size_t HASH2 = std::hash<b2Body*>()(key.CollisionBodyB);
			// clac key_hash value.
			if (HASH1 > HASH2) std::swap(HASH1, HASH2);
			return HASH1 ^ (HASH2 << 1);
		}
	};

	// mode: event: collision_pair => storage pair pointer(key), actor_unique(value) => get total(frame),first.
	using CollisionHashMap = std::unordered_map<__PsagPhyCollisionKey, PhysicsCollisionCode, __PsagPhyCollisionKeyHASH>;
	class __PsagPhyContactListener :public b2ContactListener {
	public:
		std::unordered_map<PhyBodyKey, PhysicsBodyData> PhysicsDataset = {};

		CollisionHashMap PhysicsCollision = {};
		CollisionHashMap PhysicsSensor    = {};

		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;
	};

	struct PhysiceWorldData {
		// physics world & contact object.
		b2World* PhysicsWorld;
		__PsagPhyContactListener* PhysicsContact;

		PhysiceWorldData() : PhysicsWorld(nullptr), PhysicsContact(nullptr) {}
	};

	// box2d worlds bodies manager.
	class PhyEngineCoreDataset :public __PHYSICS_ENGINE_TIMESETP {
	protected:
		static std::unordered_map<std::string, PhysiceWorldData> PhysicsWorlds;
		// x:velocity_iterations, y:position_iterations.
		static Vector2T<float> PhysicsIterations;

		bool PhyBodyItemAlloc(std::string world, PhyBodyKey* rukey, PhysicsBodyConfig config);
		bool PhyBodyItemFree(std::string world, PhyBodyKey rukey);

		void PhyBodyItemResetBox(std::string world, PhyBodyKey rukey, Vector2T<float> size, float density, float friction);
		PhysicsRunState PhyBodyItemGet(std::string world, PhyBodyKey rukey);

		// mode: 1: physics_collision_list, 2: physics_sensor_list.
		std::vector<size_t> PhyBodyItemGetCollision     (std::string world, PhyBodyKey rukey, int mode = 1);
		size_t              PhyBodyItemGetCollisionFirst(std::string world, PhyBodyKey rukey, int mode = 1);

		// physics system: framework oper.
		bool PhysicsWorldCreate(std::string strkey, Vector2T<float> gravity_vector);
		bool PhysicsWorldDelete(std::string strkey);
		PhysiceWorldData* PhysicsWorldFind(std::string strkey);

		// framework global update state.
		void PhysicsSystemUpdateState();
	};
}

#endif