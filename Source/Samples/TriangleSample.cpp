#include <stdio.h>
#include "Core/EntryPoint.h"
#include "Core/App/AppBase.h"
#include "Core/Logging.h"
#include "Graphics/GraphicsInterface.h"
#include "Graphics/Platform/BaseWindow.h"
#include "Graphics/UI/UIInterface.h"
#include "Graphics/VertexDescription.h"

using namespace Graphics;
struct Vertex
{
	float x, y, z;
	float r, g, b;
};

class TriangleApp : public AppBase
{
public:
	TriangleApp(){}
	~TriangleApp(){}
	void Init();
	void Update();
	void Release();

private:
	BufferHandle m_VertexBuffer;
	GraphicsPipeline m_Pipeline;
};

void TriangleApp::Init()
{
	AppBase::Init();
	
	Vertex arr[3] =
	{
		-0.5f, -0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
		0.0f,  0.5f, 0.5f,   0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 1.0f
	};
	m_VertexBuffer = m_GraphicsInterface->CreateBuffer(BufferType::VertexBuffer, CPUAccess::None, GPUAccess::None, sizeof(Vertex) * 3, 0, &arr[0]);
	{
		GraphicsPipelineDescription pdesc = {};
		pdesc.PixelShader.ShaderEntryPoint = "PSSimple";
		pdesc.PixelShader.ShaderPath = "shadersrc:Samples/TriangleSample.hlsl";
		pdesc.PixelShader.Type = ShaderType::Pixel;

		pdesc.VertexShader.ShaderEntryPoint = "VSSimple";
		pdesc.VertexShader.ShaderPath = "shadersrc:Samples/TriangleSample.hlsl";
		pdesc.VertexShader.Type = ShaderType::Vertex;
		pdesc.VertexDescription = PosColorVertexDescription::GetDescription();

		pdesc.ColorFormats[0] = m_GraphicsInterface->GetOutputFormat();
		m_Pipeline = m_GraphicsInterface->CreateGraphicsPipeline(pdesc);
	}

	m_GraphicsInterface->FlushAndWait();
}

void TriangleApp::Update()
{
	AppBase::Update();

	m_GraphicsInterface->SetScissor(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
	m_GraphicsInterface->SetTopology(Topology::TriangleList);
	m_GraphicsInterface->SetGraphicsPipeline(m_Pipeline);
	m_GraphicsInterface->SetVertexBuffer(m_VertexBuffer, sizeof(Vertex) * 3, sizeof(Vertex));
	m_GraphicsInterface->Draw(3, 0);

	ImGui::Begin("Paco");
	ImGui::End();
}

void TriangleApp::Release()
{
	m_GraphicsInterface->ReleaseGraphicsPipeline(m_Pipeline);
	AppBase::Release();
}

TriangleApp app;
ENTRY_POINT(app, "Triangle App", false);