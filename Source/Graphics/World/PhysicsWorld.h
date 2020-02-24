#pragma once

#include "Component.h"

namespace reactphysics3d
{
	class DynamicsWorld;
	class RigidBody;
}

namespace World
{
	class RigidBodyComponent;
	class ColliderComponent;
	class PhysicsWorld
	{
	public:
		PhysicsWorld();
		~PhysicsWorld();

		void Initialize();
		void Update(float deltaTime);

		void AddRigidBody(RigidBodyComponent* rigidBodyComponent);

	private:
		reactphysics3d::DynamicsWorld* mDynamicsWorld;
	};

	class RigidBodyComponent : public Component
	{
		friend PhysicsWorld;
	public:
		RigidBodyComponent();

		// We just finished updating the physics world, now it's a good point
		// to sync the transform with the physics
		void UpdatePhysics();
		void Update(float deltaTime) {};
		// Sync back the transformations, generally, this shouldn't do anything 
		// as you shouldn't update a dynamic/kinematic object using positions (for static it is fine).
		void UpdateLate();

		struct Type
		{
			enum T
			{
				Static,
				Kinematic,
				Dynamic,
				COUNT
			};
		};

		Type::T GetBodyType()const;
		void SetBodyType(const Type::T& t);

	private:
		reactphysics3d::RigidBody* mRigidBody;
		Type::T mBodyType;
	};

	class ColliderComponent : public Component
	{
	public:
		void Update(float deltaTime) {};

	private:
	};

	class SphereColliderComponent : public ColliderComponent
	{
	public:
		void Update(float deltaTime) {};

	private:
	};
}