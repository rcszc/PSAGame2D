// psag_physics_engine. RCSZ. [middle_level_engine]

#ifndef __PSAG_PHYSICS_ENGINE_H
#define __PSAG_PHYSICS_ENGINE_H
#include "box2d/box2d.h"

#define ENABLE_LOWMODULE_GRAPHICS
#include "../psag_lowlevel_support.h"

#define PSAGM_VIR_TICKSTEP_PHY 0.042f

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
	std::vector<b2Vec2> PresetVertexGroupSqua();
	
	struct PhysicsBodyConfig {
		size_t IndexUniqueCode;
		// 2D碰撞顶点组 (封闭多边形).
		std::vector<b2Vec2> CollVertexGroup;

		Vector2T<float> PhyBoxPosition;
		Vector2T<float> PhyBoxCollisionSize;
		float           PhyBoxRotate;

		float PhyBodyDensity;
		float PhyBodyFriction;

		// true:dynamic_body, false: static_body.
		bool PhysicsModeTypeFlag;
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

	struct PhysiceWorldData {
		// physics world & body_dataset.
		b2World* PhysicsWorld;
		std::unordered_map<ResUnique, PhysicsBodyData> PhysicsDataset;

		PhysiceWorldData() : PhysicsWorld(nullptr), PhysicsDataset({}) {}
	};
	// box2d worlds bodies manager.
	class PhyEngineCoreDataset {
	protected:
		static std::unordered_map<std::string, PhysiceWorldData> PhysicsWorlds;
		// x:velocity_iterations, y:position_iterations.
		static Vector2T<float> PhysicsIterations;

		bool PhyBodyItemAlloc(std::string world, ResUnique rukey, PhysicsBodyConfig config);
		bool PhyBodyItemFree(std::string world, ResUnique rukey);

		void PhyBodyItemResetBox(std::string world, ResUnique rukey, Vector2T<float> size, float density, float friction);
		PhysicsRunState PhyBodyItemGet(std::string world, ResUnique rukey);
		size_t PhyBodyItemGetCollision(std::string world, ResUnique rukey);

		// physics system: framework oper.
		bool PhysicsWorldCreate(std::string strkey, Vector2T<float> gravity_vector);
		bool PhysicsWorldDelete(std::string strkey);
		PhysiceWorldData* PhysicsWorldFind(std::string strkey);

		// framework global update state.
		void PhysicsSystemUpdateState(float time_step);
	};
}

#endif