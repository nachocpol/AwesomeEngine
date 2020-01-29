#define NOMINMAX

#include "Model.h"
#include "Core/Logging.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <vector>

using namespace Graphics;

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

Model* Graphics::ModelFactory::LoadFromFile(std::string path, GraphicsInterface* graphicsInterface)
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

	std::string fullPath = "..\\..\\Assets\\" + path;
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

	// Lets generate the meshes
	uint32_t numMeshes = loadedScene->mNumMeshes;
	model->Meshes = new Mesh[numMeshes];
	model->NumMeshes = numMeshes;

	//std::vector<Graphics::ShadingInfo> loadedMaterials;
	//loadedMaterials.resize(numMeshes);
	for (uint32_t i = 0; i < numMeshes; i++)
	{
		const aiMesh* curLoadedMesh = loadedScene->mMeshes[i];
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
			vtx->Position[0] = curLoadedMesh->mVertices[v].x;
			vtx->Position[1] = curLoadedMesh->mVertices[v].y;
			vtx->Position[2] = curLoadedMesh->mVertices[v].z;

			minVtx = glm::min(glm::vec3(vtx->Position[0], vtx->Position[1], vtx->Position[2]), minVtx);
			maxVtx = glm::max(glm::vec3(vtx->Position[0], vtx->Position[1], vtx->Position[2]), maxVtx);

			// Normal:
			if (curLoadedMesh->HasNormals())
			{
				vtx->Normal[0] = curLoadedMesh->mNormals[v].x;
				vtx->Normal[1] = curLoadedMesh->mNormals[v].y;
				vtx->Normal[2] = curLoadedMesh->mNormals[v].z;
			}

			// Tangent:
			if (curLoadedMesh->HasTangentsAndBitangents())
			{
				vtx->Tangent[0] = curLoadedMesh->mTangents[v].x;
				vtx->Tangent[1] = curLoadedMesh->mTangents[v].x;
				vtx->Tangent[2] = curLoadedMesh->mTangents[v].x;
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
		model->Meshes[i].VertexBuffer = graphicsInterface->CreateBuffer(Graphics::BufferType::VertexBuffer, Graphics::CPUAccess::None, vtxSize, vertices.data());
		model->Meshes[i].NumVertex = numVertices;
		model->Meshes[i].VertexSize = sizeof(PosNorTanTexc_Vertex);
		model->Meshes[i].IndexBuffer = graphicsInterface->CreateBuffer(Graphics::BufferType::IndexBuffer, Graphics::CPUAccess::None, sizeof(unsigned int) * loadedIndices.size(), loadedIndices.data());
		model->Meshes[i].NumIndices = (uint32_t)loadedIndices.size();
		
		model->Meshes[i].AABB.Min = minVtx;
		model->Meshes[i].AABB.Max = maxVtx;

		// Setup material
		//loadedMaterials[i].AlbedoColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		//if (loadedScene->HasMaterials())
		//{
		//	const auto curMaterial = loadedScene->mMaterials[curLoadedMesh->mMaterialIndex];
		//
		//	aiString tpath;
		//	aiColor4D albedoColor;
		//	aiGetMaterialColor(curMaterial, AI_MATKEY_COLOR_DIFFUSE, &albedoColor);
		//	loadedMaterials[i].AlbedoColor = glm::vec4(albedoColor.r, albedoColor.g, albedoColor.b, albedoColor.a);
		//	if (curMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &tpath) == aiReturn_SUCCESS)
		//	{
		//		loadedMaterials[i].AlbedoTexture = LoadAndCreateTexture(tpath.C_Str());
		//	}
		//	tpath.Clear();
		//	// aiTextureType_HEIGHT uhhhhhhmmm nope
		//	if (curMaterial->GetTexture(aiTextureType_HEIGHT, 0, &tpath) == aiReturn_SUCCESS)
		//	{
		//		loadedMaterials[i].BumpMapTexture = LoadAndCreateTexture(tpath.C_Str());
		//	}
		//}
	}

	return model;
}

Model* Graphics::ModelFactory::LoadFromAsset(std::string path, GraphicsInterface* graphicsInterface)
{
	return nullptr;
}
