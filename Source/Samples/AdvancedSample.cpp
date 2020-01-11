#define NOMINMAX

#include "Graphics/Platform/BaseWindow.h"
#include "Core/EntryPoint.h"
#include "Core/App/AppBase.h"
#include "Graphics/GraphicsInterface.h"
#include <stdio.h>
#include "glm/ext.hpp"

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

class AdvancedApp : public AppBase
{
public:
	AdvancedApp() {}
	~AdvancedApp() {}
	void Init();
	void Update();
	void Release();

private:
	Graphics::BufferHandle vertexBuffer;
	Graphics::GraphicsPipeline pipeline;
	Graphics::GraphicsPipeline fullScreenPipeline;
	Graphics::BufferHandle appDataBuffer;
	Graphics::BufferHandle fullScreenBuffer;

	Graphics::TextureHandle mainTarget;
	Graphics::TextureHandle mainDepth;
};

void AdvancedApp::Init()
{
	AppBase::Init();

	int width = 1024;
	int height = 720;

	auto colFlags = Graphics::TextureFlags::RenderTarget;
	mainTarget = m_GraphicsInterface->CreateTexture2D(width, height, 1, 1, Graphics::Format::RGBA_8_Unorm, colFlags);

	auto depthFlags = Graphics::TextureFlags::DepthStencil;
	mainDepth = m_GraphicsInterface->CreateTexture2D(width, height, 1, 1, Graphics::Format::Depth24_Stencil8, depthFlags);

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
	vertexBuffer = m_GraphicsInterface->CreateBuffer(Graphics::VertexBuffer, Graphics::CPUAccess::None, sizeof(arr), &arr[0]);
	{
		Graphics::GraphicsPipelineDescription pdesc = {};
		pdesc.PixelShader.ShaderEntryPoint = "PSFordwardSimple";
		pdesc.PixelShader.ShaderPath = "Fordward.hlsl";
		pdesc.PixelShader.Type = Graphics::ShaderType::Pixel;

		pdesc.VertexShader.ShaderEntryPoint = "VSFordwardSimple";
		pdesc.VertexShader.ShaderPath = "Fordward.hlsl";
		pdesc.VertexShader.Type = Graphics::ShaderType::Vertex;

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
		pdesc.ColorFormats[0] = Graphics::Format::RGBA_8_Unorm;
		pdesc.DepthFormat = Graphics::Format::Depth24_Stencil8;

		pipeline = m_GraphicsInterface->CreateGraphicsPipeline(pdesc);
	}
	{
		Graphics::GraphicsPipelineDescription desc;
		desc.DepthEnabled = false;
		desc.DepthFunction = Graphics::Always;
		desc.VertexShader.ShaderEntryPoint = "VSFullScreen";
		desc.VertexShader.ShaderPath = "Common.hlsl";
		desc.VertexShader.Type = Graphics::ShaderType::Vertex;
		desc.PixelShader.ShaderEntryPoint = "PSFullScreen";
		desc.PixelShader.ShaderPath = "Common.hlsl";
		desc.PixelShader.Type = Graphics::ShaderType::Pixel;

		Graphics::VertexInputDescription::VertexInputElement eles[1] =
		{
			"POSITION",0, Graphics::Format::RGB_32_Float,0
		};
		desc.VertexDescription.NumElements = 1;
		desc.VertexDescription.Elements = eles;

		desc.ColorFormats[0] = Graphics::Format::RGBA_8_Unorm;
		desc.DepthFormat = Graphics::Format::Depth24_Stencil8;

		fullScreenPipeline = m_GraphicsInterface->CreateGraphicsPipeline(desc);

		VertexScreen vtxData[6] =
		{
			-1.0f, 1.0f,0.0f,
			1.0f, 1.0f,0.0f,
			1.0f,-1.0f,0.0f,

			-1.0f, 1.0f,0.0f,
			1.0f,-1.0f,0.0f,
			-1.0f,-1.0f,0.0f,
		};
		fullScreenBuffer = m_GraphicsInterface->CreateBuffer(Graphics::VertexBuffer, Graphics::None, sizeof(VertexScreen) * 6, &vtxData);
	}
	appDataBuffer = m_GraphicsInterface->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(AppData));

	m_GraphicsInterface->FlushAndWait();
}

void AdvancedApp::Update()
{
	AppBase::Update();

	m_Window->Update();

	m_GraphicsInterface->StartFrame();
	m_GraphicsInterface->SetScissor(0.0f, 0.0f, m_Window->GetWidth(), m_Window->GetHeight());

	// Render to screen buffer
	m_GraphicsInterface->SetTargets(1, &mainTarget, &mainDepth);
	float clear[4] = { 0.0f,0.0f,0.0f,1.0f };
	m_GraphicsInterface->ClearTargets(1, &mainTarget, clear, &mainDepth, 1.0f, 0);
	{
		m_GraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
		m_GraphicsInterface->SetGraphicsPipeline(pipeline);
		{
			AppData.View = glm::lookAtRH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			AppData.Projection = glm::perspective(glm::radians(75.0f), 1280.0f / 920.0f, 0.1f, 100.0f);

			AppData.Model = glm::mat4(1.0f);
			AppData.Model = glm::translate(AppData.Model, glm::vec3(0.0f, 0.0f, 0.0f));
			AppData.Model = glm::rotate(AppData.Model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
			AppData.Model = glm::rotate(AppData.Model, /*t*/ 1.0f * 0.2f, glm::vec3(0.0f, 1.0f, 0.0f));
			AppData.Model = glm::rotate(AppData.Model, /*t*/ 1.0f * 0.2f, glm::vec3(0.0f, 0.0f, 1.0f));
			AppData.Model = glm::scale(AppData.Model, glm::vec3(0.5f, 0.5f, 0.5f));
			AppData.DebugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		m_GraphicsInterface->SetConstantBuffer(appDataBuffer, 0, sizeof(AppData), &AppData);
		m_GraphicsInterface->SetVertexBuffer(vertexBuffer, sizeof(VertexCube) * 36, sizeof(VertexCube));
		m_GraphicsInterface->Draw(36, 0);
		{
			AppData.Model = glm::mat4(1.0f);
			AppData.Model = glm::translate(AppData.Model, glm::vec3(1.0f, 0.5f, -1.0f));
			AppData.DebugColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}
		m_GraphicsInterface->SetConstantBuffer(appDataBuffer, 0, sizeof(AppData), &AppData);
		m_GraphicsInterface->Draw(36, 0);
	}
	m_GraphicsInterface->DisableAllTargets();

	// Output to the screen
	m_GraphicsInterface->SetGraphicsPipeline(fullScreenPipeline);
	m_GraphicsInterface->SetVertexBuffer(fullScreenBuffer, sizeof(VertexScreen) * 6, sizeof(VertexScreen));
	m_GraphicsInterface->SetResource(mainTarget, 0);
	m_GraphicsInterface->Draw(6, 0);
	m_GraphicsInterface->EndFrame();
}

void AdvancedApp::Release()
{

	AppBase::Release();
}

AdvancedApp app;
ENTRY_POINT(app, "Advanced App", false);

