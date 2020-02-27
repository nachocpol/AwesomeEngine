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
	if (deltaTime == 0.0f)return;
	mScene->simulate(deltaTime/1000.0f);
	mScene->fetchResults(true);
}

void PhysicsWorld::AddRigidBody(RigidBodyComponent* rigidBodyComponent)
{
}

void PhysicsWorld::RemoveRigidBody(RigidBodyComponent* rigidBodyComponent)
{
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
	PxTransform updatedTransform = mRigidBody->getGlobalPose();

	mParent->Transform->SetPosition(updatedTransform.p.x, updatedTransform.p.y, updatedTransform.p.z);
	mParent->Transform->SetRotation(
		glm::eulerAngles(glm::fquat(updatedTransform.q.x, updatedTransform.q.y, updatedTransform.q.z, updatedTransform.q.w
	)));
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
	mRigidBody->attachShape(*collider->mColliderShape);
//	PxRigidBodyExt::updateMassAndInertia(*(PxRigidBody*)mRigidBody, 1.0f);
}

void RigidBodyComponent::RemoveCollider(ColliderComponent* collider)
{
	mRigidBody->detachShape(*collider->mColliderShape);
}

void RigidBodyComponent::CreateRigidBody()
{
	glm::vec3 pos = mParent->Transform->GetPosition();
	glm::vec3 rot = mParent->Transform->GetRotation();
	glm::fquat rotQuat = glm::quat(rot);

	PxTransform initialTransform;
	initialTransform.p = PxVec3(pos.x, pos.y, pos.z);
	initialTransform.q = PxQuat(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w);

	PhysicsWorld* physWorld = PhysicsWorld::GetInstance();

	// This creates the rigid body!
	if (mBodyType == RigidBodyComponent::Type::Static)
	{
		mRigidBody = physWorld->mPhysics->createRigidStatic(initialTransform);
	}
	else
	{
		bool kinematic = mBodyType == RigidBodyComponent::Type::Kinematic;
		mRigidBody = physWorld->mPhysics->createRigidDynamic(initialTransform);
		if (kinematic)
		{
			// TO-DO..
		}
		((PxRigidDynamic*)mRigidBody)->setMass(1);
		PxRigidBodyExt::updateMassAndInertia(*(PxRigidBody*)mRigidBody, 1.0f);
	}
	// Add it to the scene:
	physWorld->mScene->addActor(*mRigidBody);
}

void RigidBodyComponent::ReleaseRigidBody()
{
	PhysicsWorld::GetInstance()->mScene->removeActor(*mRigidBody);
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
	PhysicsWorld* phys = PhysicsWorld::GetInstance();
	mColliderShape = phys->mPhysics->createShape(
		PxBoxGeometry(mLocalExtents.x, mLocalExtents.y, mLocalExtents.z), *phys->mMaterial
	);
}

void BoxColliderComponent::SetLocalExtents(const glm::vec3& extents)
{
	PxBoxGeometry box;
	if (mColliderShape->getBoxGeometry(box))
	{
		box.halfExtents.x = extents.x;
		box.halfExtents.y = extents.y;
		box.halfExtents.z = extents.z;
		mColliderShape->setGeometry(box);
	}
	mLocalExtents = extents;
}

glm::vec3 BoxColliderComponent::GetLocalExtents() const
{
	return mLocalExtents;
}
