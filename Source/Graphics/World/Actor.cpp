#include "Actor.h"

#include "glm/ext.hpp"

using namespace World;

Actor::Actor():
	 mPosition(0.0f,0.0f,0.0f)
	,mRotation(0.0f,0.0f,0.0f)
	,mScale(1.0f,1.0f,1.0f)
	,mParent(nullptr)
{
}

Actor::~Actor()
{
}

glm::vec3 Actor::GetPosition() const
{
	return mPosition;
}

void Actor::SetPosition(const glm::vec3& position)
{
	mPosition = position;
}

void Actor::SetPosition(const float& x, const float& y, const float& z)
{
	SetPosition(glm::vec3(x, y, z));
}

void Actor::Translate(const glm::vec3 & delta)
{
	mPosition += delta;
}

glm::vec3 Actor::GetRotation() const
{
	return mRotation;
}

void Actor::SetRotation(const glm::vec3& rotation)
{
	mRotation = rotation;
}

void Actor::SetRotation(const float& x, const float& y, const float& z)
{
	SetRotation(glm::vec3(x, y, z));
}

void Actor::Rotate(float x, float y, float z)
{
	SetRotation(mRotation.x + x, mRotation.y + y, mRotation.z + z);
}

glm::vec3 Actor::GetScale() const
{
	return mScale;
}

void Actor::SetScale(const glm::vec3& scale)
{
	mScale = scale;
}

void Actor::SetScale(const float& x, const float& y, const float& z)
{
	SetScale(glm::vec3(x, y, z));
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

void Actor::Update(float deltaTime)
{
	mWorldTransform = glm::mat4(1.0f);

	mWorldTransform = glm::translate(mWorldTransform, GetPosition());

	const glm::vec3 rotation = GetRotation();
	mWorldTransform = glm::rotate(mWorldTransform, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	mWorldTransform = glm::rotate(mWorldTransform, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	mWorldTransform = glm::rotate(mWorldTransform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

	mWorldTransform = glm::scale(mWorldTransform, GetScale());

	if (mParent)
	{
		mWorldTransform = mParent->GetWorldTransform() * mWorldTransform;
	}

	UpdateBounds();

	for (const auto c : mChilds)
	{
		c->Update(deltaTime);
	}
}

glm::mat4 Actor::GetWorldTransform()const
{
	return mWorldTransform;
}

void Actor::AddChild(Actor* child)
{
	child->mParent = this;
	mChilds.push_back(child);
}
