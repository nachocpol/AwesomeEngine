#pragma once

#include "GraphicsInterface.h"
#include <map>
namespace Graphics 
{
	class Scene;

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
		bool LoadModel(const char* path, Graphics::Scene* scene);
		bool LoadTexture(const char* path, unsigned char*& outData, int& width, int& height, int& mips, Graphics::Format& format, bool calcMips);
		void FreeLoadedTexture(void* loadedData);
		Graphics::TextureHandle LoadAndCreateTexture(const char* path);
	private:
		GraphicsInterface* mGraphicsInterface;
		// We cache already loaded textures
		std::map<std::string, Graphics::TextureHandle> mLoadedTextures;
	};
}