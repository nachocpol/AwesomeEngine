#define NOMINMAX
#include "Graphics/DX12/DX12GraphicsInterface.h"
#include "Graphics/Platform/Windows/WWindow.h"
#include "Graphics/AssetImporter.h"
#include <stdio.h>
#include "glm/ext.hpp"

Graphics::GraphicsInterface* gGraphicsInterface = nullptr;
Graphics::AssetImporter* gImporter = nullptr;

struct VertexCube
{
	float x, y, z;
};

struct VertexScreen
{
	float x, y, z;
};

struct AppData
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Projection;
}AppData;

Graphics::BufferHandle vertexBuffer;
Graphics::GraphicsPipeline pipeline;
Graphics::GraphicsPipeline fullScreenPipeline;
Graphics::BufferHandle appDataBuffer;
Graphics::TextureHandle proceduralTex;
Graphics::BufferHandle fullScreenBuffer;

Graphics::TextureHandle mainTarget;
Graphics::TextureHandle mainDepth;

bool InitGraphics(Graphics::Platform::BaseWindow* window);
void InitResources();
void Resize(int widht, int height);

int main()
{
	wchar_t wdir[128];
	GetModuleFileName(NULL, wdir, 128);

	auto window = new Graphics::Platform::Windows::WWindow();
	window->Initialize("Awesome Advanced", false, 1280, 920);
	
	InitGraphics(window);
	//gImporter = new Graphics::AssetImporter(gGraphicsInterface);
	//gImporter->Load("mitsuba\\mitsuba.obj");
	InitResources();
	
	Resize(1280, 920);

	gGraphicsInterface->FlushAndWait();

	bool running = true;
	while (running)
	{
		static float t = 0.0f;
		t += 0.16f;

		window->Update();

		gGraphicsInterface->StartFrame();
		gGraphicsInterface->SetScissor(0.0f, 0.0f, window->GetWidth(), window->GetHeight());

		// Render to screen buffer
		gGraphicsInterface->SetTargets(1, &mainTarget, &mainDepth);
		float clear[4] = { 0.0f,0.0f,0.0f,1.0f };
		gGraphicsInterface->ClearTargets(1, &mainTarget, clear, &mainDepth, 1.0f, 0);
		{
			gGraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
			gGraphicsInterface->SetGraphicsPipeline(pipeline);
			{
				AppData.View = glm::lookAtRH(glm::vec3(0.0f, 3.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				AppData.Projection = glm::perspective(glm::radians(75.0f), 1280.0f / 920.0f, 0.0f, 100.0f);

				AppData.Model = glm::mat4(1.0f);
				AppData.Model = glm::translate(AppData.Model, glm::vec3(0.0f, 0.0f, 0.0f));
				//AppData.Model = glm::rotate(AppData.Model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				//AppData.Model = glm::rotate(AppData.Model, t * 0.2f, glm::vec3(0.0f, 1.0f, 0.0f));
				//AppData.Model = glm::rotate(AppData.Model, t * 0.2f, glm::vec3(0.0f, 0.0f, 1.0f));
				AppData.Model = glm::scale(AppData.Model, glm::vec3(1.0f, 1.0f, 1.0f));
			}
			gGraphicsInterface->SetConstantBuffer(appDataBuffer, 0, sizeof(AppData), &AppData);
			gGraphicsInterface->SetVertexBuffer(vertexBuffer, sizeof(VertexCube) * 36, sizeof(VertexCube));
			gGraphicsInterface->Draw(36, 0);
			{
				AppData.Model = glm::mat4(1.0f);
				AppData.Model = glm::translate(AppData.Model, glm::vec3(1.5f, 0.0f, -1.0f));
			}
			gGraphicsInterface->SetConstantBuffer(appDataBuffer, 0, sizeof(AppData), &AppData);
			gGraphicsInterface->Draw(36, 0);
		}
		gGraphicsInterface->DisableAllTargets();

		// Output to the screen
		gGraphicsInterface->SetGraphicsPipeline(fullScreenPipeline);
		gGraphicsInterface->SetVertexBuffer(fullScreenBuffer, sizeof(VertexScreen) * 6, sizeof(VertexScreen));
		gGraphicsInterface->SetTexture(mainTarget, 0);
		gGraphicsInterface->Draw(6, 0);
		gGraphicsInterface->EndFrame();

		running = !window->IsClosed();
	}
	return 1;
}

bool InitGraphics(Graphics::Platform::BaseWindow* window)
{
	gGraphicsInterface = new Graphics::DX12::DX12GraphicsInterface();
	gGraphicsInterface->Initialize(window);
	return true;
}

void InitResources()
{
	VertexCube arr[36] =
	{
		-1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,  1.0f,-1.0f, 1.0f, -1.0f,-1.0f, 1.0f,

		-1.0f, 1.0f, 1.0f, -1.0f, 1.0f,-1.0f, 1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,  1.0f,

		-1.0f,-1.0f,1.0f, 1.0f,-1.0f,1.0f, -1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,1.0f,  1.0f,-1.0f,-1.0f,  -1.0f,-1.0f,-1.0f,

		1.0f, 1.0f,-1.0f, -1.0f, 1.0f,-1.0f, -1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f, 1.0f,-1.0f, -1.0f, -1.0f,-1.0f,-1.0f,

		1.0f, 1.0f, 1.0f, 1.0f, 1.0f,-1.0f, 1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,-1.0f,-1.0f, 1.0f,-1.0f, 1.0f,

		-1.0f, 1.0f,-1.0f, -1.0f, 1.0f,1.0f,-1.0f,-1.0f,1.0f,
		-1.0f, 1.0f,-1.0f, -1.0f,-1.0f,1.0f, -1.0f,-1.0f,-1.0f
	};
	vertexBuffer = gGraphicsInterface->CreateBuffer(Graphics::VertexBuffer, Graphics::CPUAccess::None, sizeof(arr), &arr[0]);
	{
		Graphics::GraphicsPipelineDescription pdesc = {};
		pdesc.PixelShader.ShaderEntryPoint = "PSFordwardSimple";
		pdesc.PixelShader.ShaderPath = "Fordward.hlsl";
		pdesc.PixelShader.Type = Graphics::Pixel;

		pdesc.VertexShader.ShaderEntryPoint = "VSFordwardSimple";
		pdesc.VertexShader.ShaderPath = "Fordward.hlsl";
		pdesc.VertexShader.Type = Graphics::Vertex;

		Graphics::VertexInputDescription::VertexInputElement eles[1];
		eles[0].Semantic = "POSITION";
		eles[0].Idx = 0;
		eles[0].EleFormat = Graphics::Format::RGB_32_Float;
		eles[0].Offset = 0;

		// eles[1].Semantic = "COLOR";
		// eles[1].Idx = 0;
		// eles[1].EleFormat = Graphics::Format::RGB_32_Float;
		// eles[1].Offset = sizeof(float) * 3;
		// 
		// eles[2].Semantic = "UV";
		// eles[2].Idx = 0;
		// eles[2].EleFormat = Graphics::Format::RG_32_Float;
		// eles[2].Offset = sizeof(float) * 3 * 2;

		pdesc.VertexDescription.NumElements = sizeof(eles) / sizeof(Graphics::VertexInputDescription::VertexInputElement);
		pdesc.VertexDescription.Elements = eles;
		pdesc.DepthEnabled = true;
		pdesc.DepthFunction = Graphics::LessEqual;
		pdesc.DepthFormat = Graphics::Depth24_Stencil8;
		pipeline = gGraphicsInterface->CreateGraphicsPipeline(pdesc);
	}
	{
		Graphics::GraphicsPipelineDescription desc;
		desc.DepthEnabled = false;
		desc.DepthFunction = Graphics::Always;
		desc.VertexShader.ShaderEntryPoint = "VSFullScreen";
		desc.VertexShader.ShaderPath = "Common.hlsl";
		desc.VertexShader.Type = Graphics::Vertex;
		desc.PixelShader.ShaderEntryPoint = "PSFullScreen";
		desc.PixelShader.ShaderPath = "Common.hlsl";
		desc.PixelShader.Type = Graphics::Pixel;

		Graphics::VertexInputDescription::VertexInputElement eles[1] =
		{
			"POSITION",0, Graphics::Format::RGB_32_Float,0
		};
		desc.VertexDescription.NumElements = 1;
		desc.VertexDescription.Elements = eles;

		fullScreenPipeline = gGraphicsInterface->CreateGraphicsPipeline(desc);

		VertexScreen vtxData[6] =
		{
			-1.0f, 1.0f,0.0f,
			 1.0f, 1.0f,0.0f,
			 1.0f,-1.0f,0.0f,

			-1.0f, 1.0f,0.0f,
			 1.0f,-1.0f,0.0f,
			-1.0f,-1.0f,0.0f,
		};
		fullScreenBuffer = gGraphicsInterface->CreateBuffer(Graphics::VertexBuffer, Graphics::None, sizeof(VertexScreen) * 6, &vtxData);
	}
	appDataBuffer = gGraphicsInterface->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(AppData));

	{
		struct Texel
		{
			float R;
			float G;
			float B;
			float A;
		};
		Texel* texels;
		int w = 128;
		int h = 128;
		texels =(Texel*) malloc(sizeof(Texel) * w * h);
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < h; x++)
			{
				float u = (float)x / (float)w;
				float v = (float)y / (float)h;
				Texel t;
				t.R = (sin(u * 50.0f) + 1.0f) * 0.5f;
				t.G = (cos(v * 50.0f) + 1.0f) * 0.5f;
				t.B = t.A = 0.0f;
				texels[y * w + x] = t;
			}
		}
		proceduralTex = gGraphicsInterface->CreateTexture2D(w, h, 1, 1, Graphics::Format::RGBA_32_Float,Graphics::TextureFlagNone, texels);
	}
}

void Resize(int width, int height)
{
	auto colFlags = Graphics::TextureFlags::RenderTarget;
	mainTarget = gGraphicsInterface->CreateTexture2D(width, height, 1, 1, Graphics::Format::RGBA_8_Unorm, colFlags);

	auto depthFlags = Graphics::TextureFlags::DepthStencil;
	mainDepth = gGraphicsInterface->CreateTexture2D(width, height, 1, 1, Graphics::Format::Depth24_Stencil8, depthFlags);
}

