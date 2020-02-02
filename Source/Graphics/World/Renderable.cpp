#include "Renderable.h"
#include "Model.h"

using namespace World;
using namespace Graphics;

Renderable::Renderable():
	mModel(nullptr)
{
}

Renderable::~Renderable()
{
}

Graphics::Model* Renderable::GetModel() const
{
	return mModel;
}

void Renderable::SetModel(Graphics::Model* model)
{
	mModel = model;
	mWorldBS.resize(mModel->NumMeshes);
	mWorldAABB.resize(mModel->NumMeshes);
}

void Renderable::UpdateBounds()
{
	if (!mModel)
	{
		return;
	}

	for (uint32_t meshIdx = 0; meshIdx < mModel->NumMeshes; ++meshIdx)
	{
		// Should this be the other way around, as in, just transform the original aabb:
		const AABBData aabb = mModel->Meshes[meshIdx].AABB;
		glm::vec3 FrontTR = glm::vec3(aabb.Max.x, aabb.Max.y, aabb.Max.z);
		glm::vec3 FrontTL = glm::vec3(aabb.Min.x, aabb.Max.y, aabb.Max.z);
		glm::vec3 BackTR = glm::vec3(aabb.Max.x, aabb.Max.y, aabb.Min.z);
		glm::vec3 BackTL = glm::vec3(aabb.Min.x, aabb.Max.y, aabb.Min.z);
		glm::vec3 FrontBR = glm::vec3(aabb.Max.x, aabb.Min.y, aabb.Max.z);
		glm::vec3 FrontBL = glm::vec3(aabb.Min.x, aabb.Min.y, aabb.Max.z);
		glm::vec3 BackBR = glm::vec3(aabb.Max.x, aabb.Min.y, aabb.Min.z);
		glm::vec3 BackBL = glm::vec3(aabb.Min.x, aabb.Min.y, aabb.Min.z);

		const glm::mat4 worldMtx = GetWorldTransform();
		FrontTR = worldMtx * glm::vec4(FrontTR, 1.0f);
		FrontTL = worldMtx * glm::vec4(FrontTL, 1.0f);
		BackTR = worldMtx * glm::vec4(BackTR, 1.0f);
		BackTL = worldMtx * glm::vec4(BackTL, 1.0f);
		FrontBR = worldMtx * glm::vec4(FrontBR, 1.0f);
		FrontBL = worldMtx * glm::vec4(FrontBL, 1.0f);
		BackBR = worldMtx * glm::vec4(BackBR, 1.0f);
		BackBL = worldMtx * glm::vec4(BackBL, 1.0f);

		AABBData worldAABB;

		worldAABB.Min = FrontTR;
		// worldAABB.Min = glm::min(FrontTR, worldAABB.Min);
		worldAABB.Min = glm::min(FrontTL, worldAABB.Min);
		worldAABB.Min = glm::min(BackTR, worldAABB.Min);
		worldAABB.Min = glm::min(BackTL, worldAABB.Min);
		worldAABB.Min = glm::min(FrontBR, worldAABB.Min);
		worldAABB.Min = glm::min(FrontBL, worldAABB.Min);
		worldAABB.Min = glm::min(BackBR, worldAABB.Min);
		worldAABB.Min = glm::min(BackBL, worldAABB.Min);

		worldAABB.Max = FrontTR;
		// worldAABB.Max = glm::max(FrontTR, worldAABB.Max);
		worldAABB.Max = glm::max(FrontTL, worldAABB.Max);
		worldAABB.Max = glm::max(BackTR, worldAABB.Max);
		worldAABB.Max = glm::max(BackTL, worldAABB.Max);
		worldAABB.Max = glm::max(FrontBR, worldAABB.Max);
		worldAABB.Max = glm::max(FrontBL, worldAABB.Max);
		worldAABB.Max = glm::max(BackBR, worldAABB.Max);
		worldAABB.Max = glm::max(BackBL, worldAABB.Max);

		mWorldAABB[meshIdx] = worldAABB;

		// Update bounding sphere:
		BSData worldBS;
		worldBS.Center = (worldAABB.Max + worldAABB.Min) * 0.5f;
		worldBS.Radius = glm::length(worldAABB.Max - worldAABB.Min) * 0.5f;

		mWorldBS[meshIdx] = worldBS;
	}
}

AABBData World::Renderable::GetWorldAABB(uint32_t meshIdx)
{
	if (!mModel || mModel->NumMeshes <= 0 || meshIdx >= mModel->NumMeshes)
	{
		return AABBData();
	}
	return mWorldAABB[meshIdx];
}

BSData Renderable::GetWorldBS(uint32_t meshIdx)
{
	if (!mModel || mModel->NumMeshes <= 0 || meshIdx >= mModel->NumMeshes)
	{
		return BSData();
	}
	return mWorldBS[meshIdx];
}
