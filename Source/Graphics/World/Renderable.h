// Renderable.h
// A renderable is a 3D entity that will be rendered. It can also be controlled
// by the physics simulation.

#pragma once

#include "Actor.h"
#include "Graphics/GraphicsInterface.h"
#include "Core/Math.h"

#include "glm/glm.hpp"

namespace Graphics
{
	struct Model;
}

namespace reactphysics3d
{
	class RigidBody;
	class DynamicsWorld;
	class CollisionShape;
	class ProxyShape;
}

namespace World
{
	struct BodyType
	{
		enum T
		{
			Static,		// Can be moved. Does not collide with other bodies.
			Dynamic,	// Full simulation applied (forces). Collides with all other body types.
			Kinematic,  // Can be moved by applying velocities. Collides with dynamic bodies.
			COUNT
		};
	};

	class Renderable : public Actor
	{
	public:
		Renderable();
		~Renderable();

		// Override this methods so we can  inform the rigid body.
		//void SetPosition(const glm::vec3& position)override;
		//void SetRotation(const glm::vec3& rotation)override;

	public:
		Graphics::Model* GetModel()const;
		void SetModel(Graphics::Model* model);

		Type::T GetActorType() const { return Type::Renderable; }

		void UpdatePhysics()override;
		void UpdateBounds()override;
		Math::AABBData GetWorldAABB(uint32_t meshIdx);
		Math::BSData GetWorldBS(uint32_t meshIdx);

	private:
		Graphics::Model* mModel;
		std::vector<Math::BSData> mWorldBS;
		std::vector<Math::AABBData> mWorldAABB;
	};
}