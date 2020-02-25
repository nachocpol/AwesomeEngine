#pragma once

#include "Component.h"

#include "glm/glm.hpp"

namespace reactphysics3d
{
	class DynamicsWorld;
	class RigidBody;
	class ProxyShape;
	class BoxShape;
	class CollisionShape;
	class SphereShape;
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
		friend ColliderComponent;
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

		void AddCollider(ColliderComponent* collider, glm::mat4 transform = glm::mat4(1.0f));
		void RemoveCollider(ColliderComponent* collider);

	private:
		reactphysics3d::RigidBody* mRigidBody;
		Type::T mBodyType;
		float mMass;
	};

	class ColliderComponent : public Component
	{
		friend RigidBodyComponent;
	public:
		ColliderComponent();
		void Update(float deltaTime) {};

	protected:
		virtual reactphysics3d::CollisionShape* GetCollisionShape() { return nullptr; }
		RigidBodyComponent* mRigidBodyOwner;
		reactphysics3d::ProxyShape* mProxyShape;
	};

	class SphereColliderComponent : public ColliderComponent
	{
	public:
		SphereColliderComponent();
		void Update(float deltaTime) {};

	protected:
		reactphysics3d::CollisionShape* GetCollisionShape()override;

	private:
		reactphysics3d::SphereShape* mSphereShape;
	};

	class BoxColliderComponent : public ColliderComponent
	{
	public:
		BoxColliderComponent();
		void Update(float deltaTime) {};

		void SetLocalExtents(const glm::vec3& extents);
		glm::vec3 GetLocalExtents()const;

	protected:
		reactphysics3d::CollisionShape* GetCollisionShape()override;

	private:
		reactphysics3d::BoxShape* mBoxShape;
		glm::vec3 mLocalExtents;
	};

}