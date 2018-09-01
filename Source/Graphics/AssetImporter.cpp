#include "AssetImporter.h"
#include "GraphicsInterface.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "cro_mipmap.h"

#include "Graphics/World/Scene.h"

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
		std::string fullPath = "..\\..\\Assets\\Meshes\\" + std::string(path);

		unsigned int assimpFlags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_GenSmoothNormals;
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

		// Lets generate the meshes
		unsigned int numMeshes = loadedScene->mNumMeshes;
		std::vector<Graphics::Mesh> loadedMeshes;
		loadedMeshes.resize(numMeshes);
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

			loadedMeshes[i].VertexBuffer = mGraphicsInterface->CreateBuffer(Graphics::BufferType::VertexBuffer, Graphics::CPUAccess::None, vtxSize, vertices.data());
			loadedMeshes[i].NumVertex = numVertices;
			loadedMeshes[i].VertexSize = vtxSize;
			//loadedMeshes[i].IndexBuffer = mGraphicsInterface->CreateBuffer(Graphics::BufferType::IndexBuffer, Graphics::CPUAccess::None, sizeof(unsigned int) * loadedIndices.size(), loadedIndices.data());
			loadedMeshes[i].NumIndices = loadedIndices.size();
		}

		// std::string fullPath = "..\\..\\Assets\\Meshes\\" + std::string(path);
		// tinyobj::attrib_t attribs;
		// std::vector<tinyobj::shape_t> shapes;
		// std::vector<tinyobj::material_t> materials;
		// std::string log;
		// 
		// if (!tinyobj::LoadObj(&attribs, &shapes, &materials, &log, fullPath.c_str()))
		// {
		// 	std::cout << log << std::endl;
		// }
		// if (log.size())
		// {
		// 	// std::cout << log << std::endl;
		// }
		// 
		// std::vector<std::vector<FullVertex>> vertexDataShapes;
		// 
		// vertexDataShapes.resize(shapes.size());
		// 
		// int shapeIdx = 0;
		// for (auto shapeIt = shapes.begin(); shapeIt != shapes.end(); ++shapeIt)
		// {
		// 	vertexDataShapes[shapeIdx].resize(shapeIt->mesh.indices.size());
		// 	FullVertex* cur = &vertexDataShapes[shapeIdx][0];
		// 	for (int i = 0; i < shapeIt->mesh.indices.size(); i++)
		// 	{
		// 		const auto curIdx = shapeIt->mesh.indices[i];
		// 
		// 		cur->Position.x = attribs.vertices[3 * curIdx.vertex_index + 0];
		// 		cur->Position.y = attribs.vertices[3 * curIdx.vertex_index + 1];
		// 		cur->Position.z = attribs.vertices[3 * curIdx.vertex_index + 2];
		// 
		// 		cur->Normal = glm::vec3(0.0f, 0.0f, 0.0f);
		// 		if (curIdx.normal_index != -1)
		// 		{
		// 			cur->Normal.x = attribs.normals[3 * curIdx.normal_index + 0];
		// 			cur->Normal.y = attribs.normals[3 * curIdx.normal_index + 1];
		// 			cur->Normal.z = attribs.normals[3 * curIdx.normal_index + 2];
		// 			cur->Normal = glm::normalize(cur->Normal);
		// 		}
		// 		
		// 		cur->Texcoords = glm::vec2(0.0f, 0.0f);
		// 		if (curIdx.texcoord_index != -1)
		// 		{
		// 			cur->Texcoords.x = attribs.texcoords[2 * curIdx.texcoord_index + 0];
		// 			cur->Texcoords.y = attribs.texcoords[2 * curIdx.texcoord_index + 1];
		// 		}
		// 
		// 		cur++;
		// 	}
		// 	shapeIdx++;
		// }
		// 
		// // Calculate tangents
		// for (int i=0;i<vertexDataShapes.size();i++)
		// {
		// 	auto& curVtxBuffer = vertexDataShapes[i];
		// 	for (int i = 0; i < curVtxBuffer.size(); i += 3)
		// 	{
		// 		FullVertex* curVtx = &curVtxBuffer[i];
		// 
		// 		// vtx0
		// 		auto p0 = curVtx->Position;
		// 		auto tc0 = curVtx->Texcoords;
		// 		curVtx++;
		// 
		// 		// vtx1
		// 		auto p1 = curVtx->Position;
		// 		auto tc1 = curVtx->Texcoords;
		// 		curVtx++;
		// 
		// 		// vtx2
		// 		auto p2 = curVtx->Position;
		// 		auto tc2 = curVtx->Texcoords;
		// 		curVtx++;
		// 
		// 		glm::vec3 edge1		= p1 - p0;
		// 		glm::vec3 edge2		= p2 - p0;
		// 		glm::vec2 deltaUV1	= tc1 - tc0;
		// 		glm::vec2 deltaUV2	= tc2 - tc0;
		// 		float f				= 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		// 		glm::vec3 triTangent;
		// 		triTangent.x	= f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		// 		triTangent.y	= f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		// 		triTangent.z	= f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		// 		triTangent		= glm::normalize(triTangent);
		// 
		// 		curVtxBuffer[i + 0].Tangent = glm::vec4(triTangent,0.0f);
		// 		curVtxBuffer[i + 1].Tangent = glm::vec4(triTangent,0.0f);	
		// 		curVtxBuffer[i + 2].Tangent = glm::vec4(triTangent,0.0f);
		// 	}
		// }

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

		// Create the meshes 
		//auto eleSize = sizeof(FullVertex);
		//auto numMeshes = shapes.size();
		//Mesh* loadedMeshes = new Mesh[numMeshes];
		//for (int i = 0; i < numMeshes; i++)
		//{
		//	auto numEles = vertexDataShapes[i].size();
		//	loadedMeshes[i].VertexBuffer = mGraphicsInterface->CreateBuffer
		//	(
		//		BufferType::VertexBuffer, 
		//		CPUAccess::None, 
		//		numEles * eleSize, 
		//		vertexDataShapes[i].data()
		//	);
		//	loadedMeshes[i].VertexSize = eleSize;
		//	loadedMeshes[i].NumVertex = numEles;
		//	loadedMeshes[i].SphericalBounds = boundingSpheres[i];
		//}
		//
		//// Add the meshes to the scene 
		for (int i = 0; i < numMeshes; i++)
		{
			auto actor = scene->AddActor();
			actor->AMesh = loadedMeshes[i];
		
			//tinyobj::material_t* mat;
			//if (!materials.empty())
			//{
			//	mat = &materials[shapes[i].mesh.material_ids[0]];
			//	if (!mat->diffuse_texname.empty())
			//	{
			//		LoadAndCreateTexture(mat->diffuse_texname.c_str(), actor->ShadeInfo.AlbedoTexture);
			//	}
			//	if (!mat->bump_texname.empty())
			//	{
			//		LoadAndCreateTexture(mat->bump_texname.c_str(), actor->ShadeInfo.BumpMapTexture);
			//	}
			//	actor->ShadeInfo.AlbedoColor = glm::vec4(mat->diffuse[0], mat->diffuse[1], mat->diffuse[2], 1.0f);
			//}
			//// Make obvious that we dont have a material
			//else
			{
				actor->ShadeInfo.AlbedoColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
			}
		}

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

	bool AssetImporter::LoadAndCreateTexture(const char* path, Graphics::TextureHandle& outHandle)
	{
		// Check if the texture is already loaded
		if (mLoadedTextures.find(path) != mLoadedTextures.end())
		{
			outHandle = mLoadedTextures[path];
			return true;
		}
		// ...otherwise load it
		unsigned char* tData = nullptr;
		int x, y,m;
		Graphics::Format format;
		if (LoadTexture(path, tData, x, y, m, format, true))
		{
			outHandle = mGraphicsInterface->CreateTexture2D(x, y, m, 1, format, Graphics::TextureFlagNone, tData);
			mLoadedTextures[path] = outHandle;
			FreeLoadedTexture(tData);
			return true;
		}

		return false;
	}
}