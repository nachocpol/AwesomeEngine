#include "glm/glm.hpp"

#include "Core/EntryPoint.h"
#include "Core/App/AppBase.h"

#include "Graphics/GraphicsInterface.h"
#include "Graphics/VertexDescription.h"
#include "Graphics/Platform/Windows/WWindow.h"

#include "../Assets/Shaders/HLSL/Samples/CubeSample.hlsl"

using namespace Graphics;

class CubeApp : public AppBase
{
	void Init();
	void Update();
	void Release();

private:
	GraphicsPipeline m_CubePSO;
	BufferHandle m_CubeVB;
	BufferHandle m_CubeIB;

	Declarations::CubeConstants m_CubeConstants;
	BufferHandle m_CubeConstantsBuffer;
};

void CubeApp::Init()
{
	AppBase::Init();

	GraphicsPipelineDescription psoDesc = {};
	{
		psoDesc.VertexShader.ShaderPath = "shadersrc:Samples/CubeSample.hlsl";
		psoDesc.VertexShader.ShaderEntryPoint = "VSCube";
		psoDesc.VertexShader.Type = ShaderType::Vertex;

		psoDesc.PixelShader.ShaderPath = "shadersrc:Samples/CubeSample.hlsl";
		psoDesc.PixelShader.ShaderEntryPoint = "PSCube";
		psoDesc.PixelShader.Type = ShaderType::Pixel;

		psoDesc.ColorFormats[0] = m_GraphicsInterface->GetOutputFormat();

		psoDesc.CullMode = FaceCullMode::Front;

		psoDesc.VertexDescription = PosVertexDescription::GetDescription();
	}
	m_CubePSO = m_GraphicsInterface->CreateGraphicsPipeline(psoDesc);

	PosVertexDescription vtxData[] = {
		 1.0f,-1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f
	};
	
	m_CubeVB = m_GraphicsInterface->CreateBuffer(
		BufferType::VertexBuffer, CPUAccess::None, GPUAccess::Read, 
		sizeof(PosVertexDescription) * 8, 0, (void*)vtxData
	);

	int idxData[] = {
		1,2,3, 7,6,5,
		4,5,1, 5,6,2,
		2,6,7, 0,3,7,
		0,1,3, 4,7,5,
		0,4,1, 1,5,2,
		3,2,7, 4,0,7
	};

	m_CubeIB = m_GraphicsInterface->CreateBuffer(
		BufferType::IndexBuffer, CPUAccess::None, GPUAccess::Read, sizeof(int) * 36, 0, idxData
	);

	m_CubeConstantsBuffer = m_GraphicsInterface->CreateBuffer(
		Graphics::BufferType::ConstantBuffer, CPUAccess::None, GPUAccess::Read, sizeof(m_CubeConstants)
	);
}

void CubeApp::Update()
{
	AppBase::Update();

	glm::mat4x4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4x4 proj = glm::perspectiveFov(90.0f, (float)m_Window->GetWidth(), (float)m_Window->GetHeight(), 0.1f, 100.0f);

	m_CubeConstants.Model = glm::rotate(glm::mat4x4(), TotalTime, glm::vec3(1.0f, 0.0f, 0.0f));
	m_CubeConstants.Model = glm::rotate(m_CubeConstants.Model, TotalTime, glm::vec3(0.0f, 1.0f, 0.0f));
	m_CubeConstants.Model = glm::rotate(m_CubeConstants.Model, -TotalTime, glm::vec3(0.0f, 0.0f, 1.0f));

	m_CubeConstants.ViewProjection = proj * view;

	m_GraphicsInterface->SetScissor(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

	m_GraphicsInterface->SetConstantBuffer(m_CubeConstantsBuffer, Declarations::kCubeConstantsSlot, sizeof(m_CubeConstants), &m_CubeConstants);
	
	m_GraphicsInterface->SetTopology(Topology::TriangleList);
	m_GraphicsInterface->SetGraphicsPipeline(m_CubePSO);
	m_GraphicsInterface->SetVertexBuffer(m_CubeVB, sizeof(PosVertexDescription) * 8, sizeof(PosVertexDescription));
	m_GraphicsInterface->SetIndexBuffer(m_CubeIB, 36 * 8, Format::R_32_Uint); // MMM format? look into how it works with the low level code
	m_GraphicsInterface->DrawIndexed(36);
}

void CubeApp::Release()
{
	AppBase::Release();
}

CubeApp app;
ENTRY_POINT(app, "CubeSample", false);