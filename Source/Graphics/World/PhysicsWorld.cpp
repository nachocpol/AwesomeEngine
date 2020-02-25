#include "PhysicsWorld.h"
#include "TransformComponent.h"
#include "Actor.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#include "reactphysics3d.h"

using namespace World;
using namespace reactphysics3d;

PhysicsWorld::PhysicsWorld():
	mDynamicsWorld(nullptr)
{

}

PhysicsWorld::~PhysicsWorld()
{

}

void PhysicsWorld::Initialize()
{
	// Setup the world:
	Vector3 kGravity(0.0f, -0.81f, 0.0f);
	WorldSettings kPhysicsSettings = {};
	mDynamicsWorld = new DynamicsWorld(kGravity, kPhysicsSettings);
}

void PhysicsWorld::Update(float deltaTime)
{
	// Start by stepping the physics world. deltaTime is not constant... We probably want to fix that!
	mDynamicsWorld->update(deltaTime / 1000.0f);
}

void PhysicsWorld::AddRigidBody(RigidBodyComponent* rigidBodyComponent)
{
	TransformComponent* transform = rigidBodyComponent->GetParent()->Transform;

	// Retrieve initial transform for the rigid body:
	Vector3 initialPos = Vector3(transform->GetPosition().x, transform->GetPosition().y, transform->GetPosition().z);
	glm::fquat quat = glm::fquat(transform->GetRotation());
	Quaternion initialRot = Quaternion(quat.x, quat.y, quat.z, quat.w);
	Transform rbTransform;
	rbTransform.setPosition(initialPos);
	rbTransform.setOrientation(initialRot);

	// Create the rigid body:
	rigidBodyComponent->mRigidBody = mDynamicsWorld->createRigidBody(rbTransform);
	rigidBodyComponent->SetBodyType(RigidBodyComponent::Type::Dynamic);
}

RigidBodyComponent::RigidBodyComponent():
	 mRigidBody(nullptr)
	,mBodyType(Type::Dynamic)
{
}

void World::RigidBodyComponent::UpdatePhysics()
{
	if (mRigidBody)
	{
		Transform transform = mRigidBody->getTransform();
		mParent->Transform->SetPosition(transform.getPosition().x, transform.getPosition().y, transform.getPosition().z);
		mParent->Transform->SetRotation(glm::eulerAngles(glm::fquat(
			transform.getOrientation().x, transform.getOrientation().y, transform.getOrientation().z, transform.getOrientation().w)
		));
	}
}

void RigidBodyComponent::UpdateLate()
{
	if (mRigidBody)
	{
		Vector3 pos = Vector3(mParent->Transform->GetPosition().x, mParent->Transform->GetPosition().y, mParent->Transform->GetPosition().z);
		glm::fquat quat = glm::fquat(mParent->Transform->GetRotation());
		Quaternion rot= Quaternion(quat.x, quat.y, quat.z, quat.w);
		Transform rbTransform;
		rbTransform.setPosition(pos);
		rbTransform.setOrientation(rot);
	}
}

RigidBodyComponent::Type::T World::RigidBodyComponent::GetBodyType() const
{
	return mBodyType;
}

void World::RigidBodyComponent::SetBodyType(const RigidBodyComponent::Type::T& t)
{
	if (t != mBodyType)
	{
		mBodyType = t;
		mRigidBody->setType(t == Type::Dynamic ? BodyType::DYNAMIC : (t == Type::Kinematic ? BodyType::KINEMATIC : BodyType::STATIC));
	}
}

void RigidBodyComponent::AddCollider(ColliderComponent* collider, float mass, glm::mat4 transform)
{
	Transform colliderTransform;
	colliderTransform.setFromOpenGL(&transform[0][0]);

	ProxyShape* proxy = mRigidBody->addCollisionShape(collider->GetCollisionShape(), colliderTransform, mass);
}

void RigidBodyComponent::RemoveCollider(ColliderComponent* collider)
{
}

SphereColliderComponent::SphereColliderComponent() :
	 mSphereShape(nullptr)
{
	mSphereShape = new SphereShape(1.0f);
}

reactphysics3d::CollisionShape* SphereColliderComponent::GetCollisionShape()
{
	return mSphereShape;
}

BoxColliderComponent::BoxColliderComponent():
	mBoxShape(nullptr)
{
	// 1x1x1 box
	mBoxShape = new BoxShape(Vector3(0.5f, 0.5f, 0.5f));
}

reactphysics3d::CollisionShape* BoxColliderComponent::GetCollisionShape()
{
	return mBoxShape;
}
