#include "Actor.h"

using namespace World;

Actor::Actor():
	mParent(nullptr)
{
}

Actor::~Actor()
{
}

uint32_t Actor::GetNumChilds() const
{
	return (uint32_t)mChilds.size();
}

Actor* Actor::GetChild(uint32_t index)
{
	return mChilds[index];
}

const std::vector<Actor*>& Actor::GetChilds() const
{
	return mChilds;
}

Actor* World::Actor::GetParent() const
{
	return mParent;
}

void World::Actor::UpdatePhysics()
{
	for (const auto c : mChilds)
	{
		c->UpdatePhysics();
	}
}

void Actor::Update(float deltaTime)
{
	for (Component* component : mComponents)
	{
		component->Update(deltaTime);
	}

	UpdateBounds();

	for (const auto c : mChilds)
	{
		c->Update(deltaTime);
	}
}

//glm::mat4 Actor::GetWorldTransform()const
//{
//	return mWorldTransform;
//}

void Actor::AddChild(Actor* child)
{
	child->mParent = this;
	mChilds.push_back(child);
}
