#include "PhysicsWorld.h"
#include "TransformComponent.h"
#include "Actor.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#include "PxPhysicsAPI.h"

using namespace World;

PhysicsWorld::PhysicsWorld()
{

}

PhysicsWorld::~PhysicsWorld()
{

}

void PhysicsWorld::Initialize()
{

}

void PhysicsWorld::Update(float deltaTime)
{
	
}

void PhysicsWorld::AddRigidBody(RigidBodyComponent* rigidBodyComponent)
{

}

RigidBodyComponent::RigidBodyComponent() :
	mBodyType(Type::Dynamic)
	,mMass(1.0f)
{
}

void RigidBodyComponent::UpdatePhysics()
{
}

void RigidBodyComponent::UpdateLate()
{
}

RigidBodyComponent::Type::T RigidBodyComponent::GetBodyType() const
{
	return mBodyType;
}

void RigidBodyComponent::SetBodyType(const RigidBodyComponent::Type::T& t)
{
}


void RigidBodyComponent::RemoveCollider(ColliderComponent* collider)
{
}

ColliderComponent::ColliderComponent() :
	mRigidBodyOwner(nullptr)
{
}

SphereColliderComponent::SphereColliderComponent() 
{
}


BoxColliderComponent::BoxColliderComponent():
	mLocalExtents(0.5f,0.5f,0.5f)
{
}

void BoxColliderComponent::SetLocalExtents(const glm::vec3& extents)
{
}

glm::vec3 BoxColliderComponent::GetLocalExtents() const
{
	return mLocalExtents;
}
