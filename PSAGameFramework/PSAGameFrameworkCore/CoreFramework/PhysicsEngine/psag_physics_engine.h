// psag_physics_engine. RCSZ. [middle_level_engine]

#ifndef __PSAG_PHYSICS_ENGINE_H
#define __PSAG_PHYSICS_ENGINE_H
#include "box2d/box2d.h"

#define ENABLE_LOWMODULE_GRAPHICS
#include "../psag_lowlevel_support.h"

#define PSAGM_VIR_TICKSTEP_PHY 0.042f

// physics engine: box2d. [20240518]
namespace PhysicsEngine {
#define PSAGM_PHYENGINE_LABEL "PSAG_PHYSICS"

	struct PhysicsBodyData {
		size_t FindUniqueCode;

		b2Body* Box2dGroundPointer;
		b2Body* Box2dActorPointer;

		PhysicsBodyData() : FindUniqueCode(NULL), Box2dGroundPointer(nullptr), Box2dActorPointer(nullptr) {}
		PhysicsBodyData(size_t code, b2Body* ground, b2Body* actor) : 
			FindUniqueCode(code), Box2dGroundPointer(ground), Box2dActorPointer(actor)
		{}
	};

	struct PhysicsBodyConfig {
		size_t IndexUniqueCode;

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

	class PhyEngineObjectData {
	protected:
		static std::unordered_map<std::string, PhysicsBodyData> PhysicsDataset;

		static b2World* PhysicsWorld;
		static b2Body*  PhysicsWorldGround;

		// x:velocity_iterations, y:position_iterations.
		static Vector2T<float> PhysicsIterations;

		bool PhyBodyItemAlloc(ResUnique strkey, PhysicsBodyConfig config);
		bool PhyBodyItemFree(ResUnique strkey);

		void PhyBodyItemResetBox(ResUnique strkey, Vector2T<float> size, float density, float friction);
		PhysicsRunState PhyBodyItemGet(ResUnique strkey);
		size_t PhyBodyItemGetCollision(ResUnique strkey);

		// physics system: framework oper.
		void PhysicsObjectCreate(float width_scale);
		void PhysicsObjectDelete();
		// framework global update state.
		void PhysicsSystemUpdateState(float speed_scale = 1.0f);
	};
}

#endif