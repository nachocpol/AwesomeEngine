#include "Actor.h"

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
	return mChilds.size();
}

Actor* Actor::GetChild(uint32_t index)
{
	return mChilds[index];
}

void Actor::Update(float deltaTime)
{
	for (const auto c : mChilds)
	{
		c->Update(deltaTime);
	}
}

void Actor::AddChild(Actor* child)
{
	mChilds.push_back(child);
}
