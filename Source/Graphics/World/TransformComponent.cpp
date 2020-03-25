#include "TransformComponent.h"
#include "Actor.h"
#include "Graphics/UI/IMGUI/imgui.h"

#include "glm/ext.hpp"

using namespace World;

TransformComponent::TransformComponent():
	mScale(1.0f)
{
}

void TransformComponent::Update(float deltaTime)
{
	mWorldTransform = glm::mat4(1.0f);

	mWorldTransform = glm::translate(mWorldTransform, GetPosition());

	const glm::vec3 rotation = GetRotation();
	mWorldTransform = glm::rotate(mWorldTransform, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	mWorldTransform = glm::rotate(mWorldTransform, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	mWorldTransform = glm::rotate(mWorldTransform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

	mWorldTransform = glm::scale(mWorldTransform, GetScale());

	if (mParent && mParent->GetParent())
	{
		mWorldTransform = mParent->GetParent()->FindComponent<TransformComponent>()->GetWorldTransform() * mWorldTransform;
	}
}

void TransformComponent::RenderUI()
{
	ImGui::Text("Transform Component");
	ImGui::InputFloat3("Position", &mPosition.x);
	ImGui::InputFloat3("Rotation", &mRotation.x);
	ImGui::InputFloat3("Scale", &mScale.x);
}

glm::vec3 TransformComponent::GetPosition() const
{
	return mPosition;
}

void TransformComponent::SetPosition(const glm::vec3& position)
{
	mPosition = position;
}

void TransformComponent::SetPosition(const float& x, const float& y, const float& z)
{
	SetPosition(glm::vec3(x, y, z));
}

void TransformComponent::Translate(const glm::vec3 & delta)
{
	mPosition += delta;
}

glm::vec3 TransformComponent::GetRotation() const
{
	return mRotation;
}

void TransformComponent::SetRotation(const glm::vec3& rotation)
{
	mRotation = rotation;
}

void TransformComponent::SetRotation(const float& x, const float& y, const float& z)
{
	SetRotation(glm::vec3(x, y, z));
}

void TransformComponent::Rotate(float x, float y, float z)
{
	SetRotation(mRotation.x + x, mRotation.y + y, mRotation.z + z);
}

glm::vec3 TransformComponent::GetScale() const
{
	return mScale;
}

void TransformComponent::SetScale(const glm::vec3& scale)
{
	mScale = scale;
}

void TransformComponent::SetScale(const float& x, const float& y, const float& z)
{
	SetScale(glm::vec3(x, y, z));
}

glm::mat4 World::TransformComponent::GetWorldTransform() const
{
	return mWorldTransform;
}
