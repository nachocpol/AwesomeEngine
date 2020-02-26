#pragma once

#include "Component.h"

#include "glm/glm.hpp"

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
		void Release();

		void AddRigidBody(RigidBodyComponent* rigidBodyComponent);

	private:
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

		void RemoveCollider(ColliderComponent* collider);

	private:
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
		RigidBodyComponent* mRigidBodyOwner;
	};

	class SphereColliderComponent : public ColliderComponent
	{
	public:
		SphereColliderComponent();
		void Update(float deltaTime) {};

	protected:

	private:
	};

	class BoxColliderComponent : public ColliderComponent
	{
	public:
		BoxColliderComponent();
		void Update(float deltaTime) {};

		void SetLocalExtents(const glm::vec3& extents);
		glm::vec3 GetLocalExtents()const;

	protected:

	private:
		glm::vec3 mLocalExtents;
	};

}