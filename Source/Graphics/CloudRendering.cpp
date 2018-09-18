#include "CloudRendering.h"
#include "Graphics/UI/IMGUI/imgui.h"
#include "Graphics/Noise.h"

namespace Graphics
{
	CloudRenderer::CloudRenderer():
		mGraphicsInterface(nullptr)
	{
	}

	CloudRenderer::~CloudRenderer()
	{
	}

	bool CloudRenderer::Initialize(GraphicsInterface* graphicsInterface)
	{
		mGraphicsInterface = graphicsInterface;

		{
			ValueNoise2D noise2D;
			noise2D.Initialize(64,64);

			struct Texel { unsigned char r, g, b, a; };
			int w = 128;
			int h = 128;
			Texel texData[128 * 128];
			for (int v = 0; v < h; v++)
			{
				for (int u = 0; u < w; u++)
				{
					// unsigned char r = (unsigned char)(float(u) / float(w) * 255.0f);
					// unsigned char g = (unsigned char)(float(v) / float(h) * 255.0f);
					unsigned char n0 = unsigned char(noise2D.Fbm(u*0.15f,v*0.15f,5) * 255.0f);
					unsigned char n = (n0);
					texData[u + v * w] = Texel{ n,n,n, 255 };
				}
			}
			mTestTexture = mGraphicsInterface->CreateTexture2D(w, h, 1, 1, Graphics::Format::RGBA_8_Unorm, Graphics::TextureFlagNone, texData);
		}
		return false;
	}

	void CloudRenderer::Draw(float dt)
	{
	}

	void CloudRenderer::ShowDebug()
	{
		ImGui::Begin("Clouds");
		ImGui::Text("Base value noise");
		ImGui::Image((ImTextureID)mTestTexture.Handle, ImVec2(512, 512));
		ImGui::Separator();
		ImGui::End();
	}
}
