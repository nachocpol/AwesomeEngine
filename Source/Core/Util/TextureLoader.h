#pragma once

#include <string>
#include <stdint.h>

#include "Graphics/GraphicsInterface.h" // For format, maybe move format to types file

namespace Core
{
	namespace Texture
	{
		struct TextureData
		{
			uint16_t m_Width;
			uint16_t m_Height;
			Graphics::Format m_Format;
			void* m_Data; // Owned by caller
		};

		bool LoadFromFile(const std::string& path, TextureData& loadedTexture);
		
		
		void CleanupIntermediate(TextureData& loadedTexture);
	}
}