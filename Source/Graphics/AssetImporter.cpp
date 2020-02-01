#include "AssetImporter.h"
#include "GraphicsInterface.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include "Graphics/World/SceneGraph.h"

#include "DirectXTex.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <stdint.h>


#pragma optimize("",off)

namespace Graphics
{
	AssetImporter::AssetImporter(GraphicsInterface* graphics):
		mGraphicsInterface(graphics)
	{
	}

	AssetImporter::~AssetImporter()
	{
	}

	bool AssetImporter::LoadModel(const char* path,Graphics::Scene* scene)
	{
		/*
		std::string fullPath = "..\\..\\Assets\\Meshes\\" + std::string(path);

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
			std::cout << aImporter.GetErrorString() << std::endl;
			return false;
		}
		std::cout << aImporter.GetErrorString();

		// Lets generate the meshes
		unsigned int numMeshes = loadedScene->mNumMeshes;
		std::vector<Graphics::Mesh> loadedMeshes;
		std::vector<Graphics::ShadingInfo> loadedMaterials;
		loadedMeshes.resize(numMeshes);
		loadedMaterials.resize(numMeshes);
		for (int i = 0; i < numMeshes; i++)
		{
			const aiMesh* curLoadedMesh = loadedScene->mMeshes[i];
			unsigned int numVertices = curLoadedMesh->mNumVertices;
			unsigned int vtxSize = sizeof(FullVertex) * numVertices;
			std::vector<FullVertex> vertices;
			vertices.resize(numVertices);
			FullVertex* vtx = &vertices[0];

			// Load vertex data
			for (int v = 0; v < numVertices; v++)
			{
				vtx->Position = glm::vec3(curLoadedMesh->mVertices[v].x, curLoadedMesh->mVertices[v].y, curLoadedMesh->mVertices[v].z );
				vtx->Normal = glm::vec3(curLoadedMesh->mNormals[v].x, curLoadedMesh->mNormals[v].y, curLoadedMesh->mNormals[v].z);
				if (curLoadedMesh->HasTangentsAndBitangents())
				{
					vtx->Tangent = glm::vec3(curLoadedMesh->mTangents[v].x, curLoadedMesh->mTangents[v].y, curLoadedMesh->mTangents[v].z);
					vtx->Bitangent = glm::vec3(curLoadedMesh->mBitangents[v].x, curLoadedMesh->mBitangents[v].y, curLoadedMesh->mBitangents[v].z);
				}
				if (curLoadedMesh->HasTextureCoords(0)) // Note that we ignore all the other uv channels
				{
					vtx->Texcoords = glm::vec2(curLoadedMesh->mTextureCoords[0][v].x, curLoadedMesh->mTextureCoords[0][v].y);
				}

				vtx++;
			}

			// Load indices
			assert(curLoadedMesh->mFaces[0].mNumIndices == 3);
			aiFace* curFace = &curLoadedMesh->mFaces[0];
			std::vector<unsigned int> loadedIndices;
			loadedIndices.resize(curLoadedMesh->mNumFaces * 3);
			unsigned int* curIdx = loadedIndices.data();
			for (int f = 0; f < curLoadedMesh->mNumFaces; f++)
			{
				*curIdx = curFace->mIndices[0]; curIdx++;
				*curIdx = curFace->mIndices[1]; curIdx++;
				*curIdx = curFace->mIndices[2]; curIdx++;
				curFace++;
			}
			
			// Setup the Mesh
			loadedMeshes[i].VertexBuffer = mGraphicsInterface->CreateBuffer(Graphics::BufferType::VertexBuffer, Graphics::CPUAccess::None, vtxSize, vertices.data());
			loadedMeshes[i].NumVertex = numVertices;
			loadedMeshes[i].VertexSize = sizeof(FullVertex);
			loadedMeshes[i].IndexBuffer = mGraphicsInterface->CreateBuffer(Graphics::BufferType::IndexBuffer, Graphics::CPUAccess::None, sizeof(unsigned int) * loadedIndices.size(), loadedIndices.data());
			loadedMeshes[i].NumIndices = loadedIndices.size();

			// Setup material
			loadedMaterials[i].AlbedoColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
			if (loadedScene->HasMaterials())
			{
				const auto curMaterial = loadedScene->mMaterials[curLoadedMesh->mMaterialIndex];

				aiString tpath;
				aiColor4D albedoColor;
				aiGetMaterialColor(curMaterial, AI_MATKEY_COLOR_DIFFUSE, &albedoColor);
				loadedMaterials[i].AlbedoColor = glm::vec4(albedoColor.r, albedoColor.g, albedoColor.b, albedoColor.a);
				if (curMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &tpath) == aiReturn_SUCCESS)
				{
					loadedMaterials[i].AlbedoTexture = LoadAndCreateTexture(tpath.C_Str());
				}
				tpath.Clear();
				// aiTextureType_HEIGHT uhhhhhhmmm nope
				if (curMaterial->GetTexture(aiTextureType_HEIGHT, 0, &tpath) == aiReturn_SUCCESS)
				{
					loadedMaterials[i].BumpMapTexture = LoadAndCreateTexture(tpath.C_Str());
				}
			}
		}

		// Calc bounding spheres
		// std::vector<BoundingSphere> boundingSpheres;
		// boundingSpheres.resize(shapes.size());
		// for(unsigned int i=0;i<boundingSpheres.size();i++)
		// {
		// 	for (unsigned int v = 0; v < vertexDataShapes[i].size(); v++)
		// 	{
		// 		boundingSpheres[i].Center += vertexDataShapes[i][v].Position;
		// 	}
		// 	boundingSpheres[i].Center /= vertexDataShapes[i].size();
		// 	for (unsigned int v = 0; v < vertexDataShapes[i].size(); v++)
		// 	{
		// 		float r = glm::length(boundingSpheres[i].Center - vertexDataShapes[i][v].Position);
		// 		boundingSpheres[i].Radius = max(boundingSpheres[i].Radius , r);
		// 	}
		// }

		//// Add the meshes to the scene 
		for (int i = 0; i < numMeshes; i++)
		{
			auto actor = scene->AddActor();
			actor->AMesh = loadedMeshes[i];
			actor->ShadeInfo = loadedMaterials[i];
		}
		*/
		return true;
	}

	bool AssetImporter::LoadTexture(const char* path, unsigned char*& outData, int& width, int& height,int& mips, Graphics::Format& format, bool calcMips )
	{
		std::string fullPath = "../../Assets/Textures/" + std::string(path);
		std::wstring wname = std::wstring(fullPath.begin(), fullPath.end());

		DirectX::TexMetadata metadata = {};
		DirectX::ScratchImage scratchImage;
		HRESULT res;

		if (fullPath.find(".dds") != std::string::npos)
		{
			res = DirectX::LoadFromDDSFile(wname.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratchImage);
		}
		else if (fullPath.find(".tga") != std::string::npos)
		{
			res = DirectX::LoadFromTGAFile(wname.c_str(), &metadata, scratchImage);
		}
		else
		{
			res = DirectX::LoadFromWICFile(wname.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratchImage);
		}
		if (FAILED(res))
		{
			return false;
		}

		mips = 1;
		DirectX::ScratchImage scratchWithMips;
		if (calcMips && metadata.mipLevels == 1)
		{
			DirectX::GenerateMipMaps((const DirectX::Image)*scratchImage.GetImages(), DirectX::TEX_FILTER_FANT, 0, scratchWithMips);
			mips = scratchWithMips.GetImageCount();
			
			auto image = scratchWithMips.GetImage(0, 0, 0);
			width = image->width;
			height = image->height;
			format = Graphics::Format::RGBA_8_Unorm;
			outData = (unsigned char*)malloc(scratchWithMips.GetPixelsSize());
			memcpy(outData, image->pixels, scratchWithMips.GetPixelsSize());
		}
		else
		{
			auto image = scratchImage.GetImage(0,0,0);
			width = image->width;
			height = image->height;
			format = Graphics::Format::RGBA_8_Unorm;
			outData = (unsigned char*)malloc(width * height * sizeof(uint8_t) * 4);
			memcpy(outData, image->pixels, width * height * sizeof(uint8_t) * 4);
		}

		return true;
	}

	void AssetImporter::FreeLoadedTexture(void * loadedData)
	{
		if (loadedData)
		{
			// stbi_image_free(loadedData);
		}
	}

	Graphics::TextureHandle AssetImporter::LoadAndCreateTexture(const char* path )
	{
		// Check if the texture is already loaded
		if (mLoadedTextures.find(path) != mLoadedTextures.end())
		{
			return mLoadedTextures[path];
		}
		// ...otherwise load it
		unsigned char* tData = nullptr;
		int x, y,m;
		Graphics::Format format;
		if (LoadTexture(path, tData, x, y, m, format, true))
		{
			mLoadedTextures[path] = mGraphicsInterface->CreateTexture2D(x, y, m, 1, format, Graphics::TextureFlagNone, tData);
			FreeLoadedTexture(tData);
			return mLoadedTextures[path];
		}

		return Graphics::InvalidTexture;
	}
}