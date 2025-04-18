#pragma once

#include "Component.h"
#include "Graphics/GraphicsInterface.h"

#include <string>

namespace World
{
	class ProbeComponent : public Component
	{
	public:
		ProbeComponent();
		~ProbeComponent();

		void SetSourceEnvMap(const char* path);
		void Update(float deltaTime);
		const char* GetSourcePath()const;

		Graphics::TextureHandle SourceTexture;
		Graphics::TextureHandle IrradianceTexture;
		Graphics::TextureHandle PrefilteredTexture;

	private:
		std::string mSourceEnvMap;
		Graphics::ProbeShape mShape;
		bool mInitialized;
	};
};