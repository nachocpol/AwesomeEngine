#include "AssetImporter.h"
#include "GraphicsInterface.h"
#include "Graphics/World/SceneGraph.h"
#include "Core/FileSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdint.h>

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
		return true;
	}

	bool AssetImporter::LoadTexture(const char* path, void*& outData, int& width, int& height,int& mips, Graphics::Format& format, bool calcMips )
	{
		std::string fullPath = path;
		if (!Core::FileSystem::GetInstance()->FixupPath(fullPath))
		{
			return false;
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
		void* tData = nullptr;
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