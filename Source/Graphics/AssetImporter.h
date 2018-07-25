#pragma once

#include "GraphicsInterface.h"

namespace Graphics 
{
	struct FullVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec2 Texcoords;
	};

	class GraphicsInterface;
	class AssetImporter
	{
	public:
		AssetImporter(GraphicsInterface* graphics);
		~AssetImporter();
		bool LoadModel(const char* path, Mesh*& outMeshes,uint8_t& numMeshes);
		bool LoadTexture(const char* path, unsigned char*& outData, int& width, int& height, Graphics::Format& format);
		void FreeLoadedTexture(void* loadedData);

	private:
		GraphicsInterface* mGraphicsInterface;
	};
}