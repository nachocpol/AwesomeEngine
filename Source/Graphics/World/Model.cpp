#define NOMINMAX

#include "Model.h"
#include "Actor.h"
#include "TransformComponent.h"
#include "Core/Logging.h"
#include "Core/FileSystem.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

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

	unsigned int assimpFlags = aiProcess_CalcTangentSpace | aiProcess_Triangulate;
	bool isLeftHanded = true;
	if (isLeftHanded)
	{
		assimpFlags |= aiProcess_ConvertToLeftHanded;
	}
	Assimp::Importer aImporter;
	const aiScene* loadedScene = aImporter.ReadFile(fullPath, assimpFlags);
	if (!loadedScene)
	{
		ERR(aImporter.GetErrorString());
		return false;
	}

	// TO-DO: instead of fixing mesh by rotating it, we could check the imported axis.
	//int totMeta = loadedScene->mMetaData->mNumProperties;
	//for (int i = 0; i < totMeta; ++i)
	//{
	//	aiString key =  loadedScene->mMetaData->mKeys[i];;
	//	aiMetadataEntry entry = loadedScene->mMetaData->mValues[i];
	//	glm::cos(0);
	//}

	// Lets generate the meshes
	uint32_t numMeshes = loadedScene->mNumMeshes;
	model->Meshes = new Mesh[numMeshes];
	model->NumMeshes = numMeshes;
	for (uint32_t i = 0; i < numMeshes; i++)
	{
		const aiMesh* curLoadedMesh = loadedScene->mMeshes[i];
		model->Meshes[i].Name = std::string(curLoadedMesh->mName.C_Str());
		
		uint32_t numVertices = curLoadedMesh->mNumVertices;
		uint32_t vtxSize = sizeof(PosNorTanTexc_Vertex) * numVertices;
		std::vector<PosNorTanTexc_Vertex> vertices;
		vertices.resize(numVertices);
		PosNorTanTexc_Vertex* vtx = &vertices[0];

		// Load vertex data
		glm::vec3 minVtx, maxVtx;
		minVtx = glm::vec3(FLT_MAX);
		maxVtx = glm::vec3(FLT_MIN);

		for (uint32_t v = 0; v < numVertices; v++)
		{
			// Position:
			glm::vec3 pos(curLoadedMesh->mVertices[v].x, curLoadedMesh->mVertices[v].y, curLoadedMesh->mVertices[v].z);
			pos = pos * transform;
			vtx->Position[0] = pos.x;
			vtx->Position[1] = pos.y;
			vtx->Position[2] = pos.z;

			minVtx = glm::min(glm::vec3(vtx->Position[0], vtx->Position[1], vtx->Position[2]), minVtx);
			maxVtx = glm::max(glm::vec3(vtx->Position[0], vtx->Position[1], vtx->Position[2]), maxVtx);

			// Normal:
			if (curLoadedMesh->HasNormals())
			{
				glm::vec3 nrm(curLoadedMesh->mNormals[v].x, curLoadedMesh->mNormals[v].y, curLoadedMesh->mNormals[v].z);
				nrm = nrm * transform;
				vtx->Normal[0] = nrm.x;
				vtx->Normal[1] = nrm.y;
				vtx->Normal[2] = nrm.z;
			}

			// Tangent:
			if (curLoadedMesh->HasTangentsAndBitangents())
			{
				glm::vec3 tng(curLoadedMesh->mTangents[v].x, curLoadedMesh->mTangents[v].y, curLoadedMesh->mTangents[v].z);
				tng = tng * transform;
				vtx->Tangent[0] = tng.x;
				vtx->Tangent[1] = tng.x;
				vtx->Tangent[2] = tng.x;
			}

			// Texcoord:
			if (curLoadedMesh->HasTextureCoords(0))
			{
				vtx->Texcoord[0] = curLoadedMesh->mTextureCoords[0][v].x;
				vtx->Texcoord[1] = curLoadedMesh->mTextureCoords[0][v].y;
			}


			vtx++;
		}

		// Load indices
		assert(curLoadedMesh->mFaces[0].mNumIndices == 3);
		aiFace* curFace = &curLoadedMesh->mFaces[0];
		std::vector<unsigned int> loadedIndices;
		loadedIndices.resize(curLoadedMesh->mNumFaces * 3);
		unsigned int* curIdx = loadedIndices.data();
		for (uint32_t f = 0; f < curLoadedMesh->mNumFaces; f++)
		{
			*curIdx = curFace->mIndices[0]; curIdx++;
			*curIdx = curFace->mIndices[1]; curIdx++;
			*curIdx = curFace->mIndices[2]; curIdx++;
			curFace++;
		}

		// Setup the Mesh
		model->Meshes[i].VertexBuffer = graphicsInterface->CreateBuffer(BufferType::VertexBuffer, CPUAccess::None, GPUAccess::Read, vtxSize, 0,vertices.data());
		model->Meshes[i].NumVertex = numVertices;
		model->Meshes[i].VertexSize = sizeof(PosNorTanTexc_Vertex);
		model->Meshes[i].IndexBuffer = graphicsInterface->CreateBuffer(BufferType::IndexBuffer, CPUAccess::None, GPUAccess::Read, sizeof(unsigned int) * loadedIndices.size(), 0, loadedIndices.data());
		model->Meshes[i].NumIndices = (uint32_t)loadedIndices.size();
		
		model->Meshes[i].AABB.Min = minVtx;
		model->Meshes[i].AABB.Max = maxVtx;

		// Setup default material:
		if (loadedScene->HasMaterials())
		{
			const auto curMaterial = loadedScene->mMaterials[curLoadedMesh->mMaterialIndex];
		
			aiColor4D albedoColor;
			aiGetMaterialColor(curMaterial, AI_MATKEY_COLOR_DIFFUSE, &albedoColor);
			model->Meshes[i].DefaultMaterial.BaseColor = glm::vec3(albedoColor.r, albedoColor.g, albedoColor.b); // ignoring .a !

			float shininess = 0.0f;
			aiGetMaterialFloat(curMaterial, AI_MATKEY_SHININESS, &shininess);
			model->Meshes[i].DefaultMaterial.Roughness = 1.0f - shininess;

			//aiString tpath;
			//if (curMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &tpath) == aiReturn_SUCCESS)
			//{
			//	loadedMaterials[i].AlbedoTexture = LoadAndCreateTexture(tpath.C_Str());
			//}
			//tpath.Clear();
			//// aiTextureType_HEIGHT uhhhhhhmmm nope
			//if (curMaterial->GetTexture(aiTextureType_HEIGHT, 0, &tpath) == aiReturn_SUCCESS)
			//{
			//	loadedMaterials[i].BumpMapTexture = LoadAndCreateTexture(tpath.C_Str());
			//}
		}
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