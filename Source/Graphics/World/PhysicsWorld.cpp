#include "PhysicsWorld.h"
#include "TransformComponent.h"
#include "Actor.h"
#include "Core/Logging.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

using namespace World;


PhysicsWorld::PhysicsWorld()
{
}

PhysicsWorld::~PhysicsWorld()
{

}

PhysicsWorld* World::PhysicsWorld::GetInstance()
{
	static PhysicsWorld* kInstance = nullptr;
	if (!kInstance)
	{
		kInstance = new	PhysicsWorld;
	}
	return kInstance;
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

void PhysicsWorld::RemoveRigidBody(RigidBodyComponent* rigidBodyComponent)
{
}

physx::PxDefaultCpuDispatcher* World::PhysicsWorld::GetPhyxCPUDispatcher() const
{
	return mCpuDispatcher;
}

physx::PxPhysics* World::PhysicsWorld::GetPhyx() const
{
	return mPhysics;
}

RigidBodyComponent::RigidBodyComponent(Actor* parent) :
	mBodyType(Type::Dynamic)
	,mMass(1.0f)
{
	mParent = parent;
	CreateRigidBody();
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
	if (t != mBodyType)
	{
		mBodyType = t;
		ReleaseRigidBody();
		CreateRigidBody();
	}
}


void RigidBodyComponent::AddCollider(ColliderComponent* collider)
{
}

void RigidBodyComponent::RemoveCollider(ColliderComponent* collider)
{
}

void RigidBodyComponent::CreateRigidBody()
{
}

void RigidBodyComponent::ReleaseRigidBody()
{
}

ColliderComponent::ColliderComponent() :
	 mRigidBodyOwner(nullptr)
	,mColliderShape(nullptr)
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
