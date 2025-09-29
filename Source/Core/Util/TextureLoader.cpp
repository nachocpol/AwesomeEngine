#include "TextureLoader.h"

#include "Core/FileSystem.h"
#include "Core/Logging.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool Core::Texture::LoadFromFile(const std::string& path, Core::Texture::TextureData& loadedTexture)
{
	loadedTexture = {};
	std::string fixedPath = path;	
	if (Core::FileSystem::GetInstance()->FixupPath(fixedPath))
	{
		// stb loading
		int x, y, n;
		stbi_uc* ptrImg = stbi_load(fixedPath.c_str(), &x, &y, &n, 4); // For now, always ask for 4 components
		if (ptrImg)
		{
			loadedTexture.m_Data = ptrImg;
			loadedTexture.m_Width = x;
			loadedTexture.m_Height = y;
			loadedTexture.m_Format = Graphics::Format::RGBA_8_Unorm;
			return true;
		}
		else
		{
			ERR("Could not load the requested texture: %s", fixedPath.c_str());
			return false;
		}
	}

	ERR("File can't be resolved %s", path.c_str());
	return false;
}

void Core::Texture::CleanupIntermediate(Core::Texture::TextureData& loadedTexture)
{
	if (loadedTexture.m_Data)
	{
		stbi_image_free(loadedTexture.m_Data);
	}
}
