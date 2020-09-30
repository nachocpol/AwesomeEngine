#define NOMINMAX

#include "Model.h"
#include "Actor.h"
#include "TransformComponent.h"
#include "Core/Logging.h"
#include "Core/FileSystem.h"

using namespace Graphics;
using namespace Math;

ModelFactory::ModelFactory()
{
}

ModelFactory::~ModelFactory()
{
}

ModelFactory::ModelFactory(const ModelFactory & other)
{
}

ModelFactory* Graphics::ModelFactory::Get()
{
	static ModelFactory* kModelFactory = nullptr;
	if (!kModelFactory)
	{
		kModelFactory = new ModelFactory;
	}
	return kModelFactory;
}

Model* Graphics::ModelFactory::LoadFromFile(std::string path, GraphicsInterface* graphicsInterface, glm::mat3 transform)
{
	// Check if the entry already exists:
	auto cachedRes = mModelCache.find(path);
	if (cachedRes != mModelCache.end())
	{
		return cachedRes->second;
	}

	// Create new model:
	Model* model = new Model;
	mModelCache[path] = model;

	std::string fullPath = path;
	if (!Core::FileSystem::GetInstance()->FixupPath(fullPath))
	{
		assert(false);
		return nullptr;
	}

	return model;
}

Model* Graphics::ModelFactory::LoadFromAsset(std::string path, GraphicsInterface* graphicsInterface)
{
	return nullptr;
}

using namespace World;

ModelComponent::ModelComponent():
	mModel(nullptr)
{
}

void ModelComponent::Update(float deltaTime)
{

}

void ModelComponent::UpdateLate()
{
	if (!mModel)
	{
		return;
	}
	
	if (mWorldAABB.size() != mModel->NumMeshes)
	{
		mWorldAABB.resize(mModel->NumMeshes);
		mWorldBS.resize(mModel->NumMeshes);
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
	
		const glm::mat4 worldMtx = mParent->Transform->GetWorldTransform();
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

void ModelComponent::SetModel(Model* model)
{
	mModel = model;
	mMaterials.resize(model->NumMeshes);
}

Model* ModelComponent::GetModel() const
{
	return mModel;
}

void ModelComponent::SetMaterial(const MaterialInfo& materialInfo, uint32_t meshIdx)
{
	if (mModel && meshIdx < mModel->NumMeshes)
	{
		mMaterials[meshIdx].Overriden = true;
		mMaterials[meshIdx].Material = materialInfo;
	}
	else
	{
		ERR("%s", mModel ? "Invalid meshIdx" : "Set model before assigning a material.");
	}
}

const MaterialInfo& ModelComponent::GetMaterial(uint32_t meshIdx) const
{
	if (mModel && meshIdx < mModel->NumMeshes)
	{
		return mMaterials[meshIdx].Overriden ? mMaterials[meshIdx].Material : mModel->Meshes[meshIdx].DefaultMaterial;
	}
	else
	{
		ERR("%s", mModel ? "Invalid meshIdx" : "Set model before getting a material.");
		static MaterialInfo dummy;
		return dummy;
	}
}

AABBData ModelComponent::GetWorldAABB(uint32_t meshIdx) const
{
	return mWorldAABB[meshIdx];
}

BSData ModelComponent::GetWorldBS(uint32_t meshIdx) const
{
	return mWorldBS[meshIdx];
}