#define NOMINMAX
#include "Graphics/DX12/DX12GraphicsInterface.h"
#include "Graphics/Platform/Windows/WWindow.h"

#include <stdio.h>
#include "glm/ext.hpp"

Graphics::GraphicsInterface* gGraphicsInterface = nullptr;

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
	glm::vec4 DebugColor;
}AppData;

Graphics::BufferHandle vertexBuffer;
Graphics::GraphicsPipeline pipeline;
Graphics::GraphicsPipeline fullScreenPipeline;
Graphics::BufferHandle appDataBuffer;
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
				AppData.View = glm::lookAtRH(glm::vec3(0.0f, 3.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				AppData.Projection = glm::perspective(glm::radians(75.0f), 1280.0f / 920.0f, 0.1f, 100.0f);

				AppData.Model = glm::mat4(1.0f);
				AppData.Model = glm::translate(AppData.Model, glm::vec3(0.0f, 0.0f, 0.0f));
				AppData.Model = glm::rotate(AppData.Model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				AppData.Model = glm::rotate(AppData.Model, t * 0.2f, glm::vec3(0.0f, 1.0f, 0.0f));
				AppData.Model = glm::rotate(AppData.Model, t * 0.2f, glm::vec3(0.0f, 0.0f, 1.0f));
				AppData.Model = glm::scale(AppData.Model, glm::vec3(1.0f, 1.0f, 1.0f));
				AppData.DebugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			}
			gGraphicsInterface->SetConstantBuffer(appDataBuffer, 0, sizeof(AppData), &AppData);
			gGraphicsInterface->SetVertexBuffer(vertexBuffer, sizeof(VertexCube) * 36, sizeof(VertexCube));
			gGraphicsInterface->Draw(36, 0);
			{
				AppData.Model = glm::mat4(1.0f);
				AppData.Model = glm::translate(AppData.Model, glm::vec3(1.0f, 0.5f, -1.0f));
				AppData.DebugColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
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
}

void Resize(int width, int height)
{
	auto colFlags = Graphics::TextureFlags::RenderTarget;
	mainTarget = gGraphicsInterface->CreateTexture2D(width, height, 1, 1, Graphics::Format::RGBA_8_Unorm, colFlags);

	auto depthFlags = Graphics::TextureFlags::DepthStencil;
	mainDepth = gGraphicsInterface->CreateTexture2D(width, height, 1, 1, Graphics::Format::Depth24_Stencil8, depthFlags);
}

