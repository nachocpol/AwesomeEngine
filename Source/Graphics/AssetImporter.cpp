#include "AssetImporter.h"
#include "GraphicsInterface.h"

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