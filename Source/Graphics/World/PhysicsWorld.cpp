#include "PhysicsWorld.h"
#include "TransformComponent.h"
#include "Actor.h"
#include "Core/Logging.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#include "PxPhysicsAPI.h"
#include "PxFiltering.h"
#include "extensions/PxRigidBodyExt.h"

using namespace World;
using namespace physx;


class PhysicsAllocator : public PxAllocatorCallback
{
public:
	void* allocate(size_t size, const char*, const char*, int)
	{
		return _aligned_malloc(size, 16);
	}

	void deallocate(void* ptr)
	{
		_aligned_free(ptr);
	}
};
static PhysicsAllocator kAllocator;

class PhysicsCallback : public PxErrorCallback
{
public:
	void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		INFO(message);
	}
};
static PhysicsCallback kCallback;

PhysicsWorld::PhysicsWorld()
{
}

PhysicsWorld::~PhysicsWorld()
{

}

void PhysicsWorld::Initialize()
{
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, kAllocator, kCallback);
	
	mPvd = PxCreatePvd(*mFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
	mPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	 
	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(), true, 0);

	mCpuDispatcher = PxDefaultCpuDispatcherCreate(2);
	
	PxSceneDesc sceneDesc = PxSceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = mCpuDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	mScene = mPhysics->createScene(sceneDesc);

	PxPvdSceneClient* client = mScene->getScenePvdClient();
	if (client)
	{
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}

void PhysicsWorld::Update(float deltaTime)
{
	mScene->simulate(deltaTime);
	mScene->fetchResults(true);
}

void PhysicsWorld::AddRigidBody(RigidBodyComponent* rigidBodyComponent)
{
	// This creates the rigid body!
	if (rigidBodyComponent->GetBodyType() == RigidBodyComponent::Type::Static)
	{
		rigidBodyComponent->mRigidBody = mPhysics->createRigidStatic(PxTransform());
	}
	else
	{
		bool kinematic = rigidBodyComponent->GetBodyType() == RigidBodyComponent::Type::Kinematic;
		rigidBodyComponent->mRigidBody = mPhysics->createRigidDynamic(PxTransform());
		if (kinematic)
		{
			// TO-DO..
		}
	}

	//rigidBodyComponent->mRigidBody->setMass(1);
	PxRigidBodyExt::updateMassAndInertia(*(PxRigidBody*)rigidBodyComponent->mRigidBody, 1.0f);
	// Add it to the scene:
	mScene->addActor(*rigidBodyComponent->mRigidBody);
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
