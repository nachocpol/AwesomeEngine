#include "Core/App/AppBase.h"
#include "Core/EntryPoint.h"
#include "Core/FileSystem.h"
#include "Core/Logging.h"
#include "Core/Util/OBJLoader.h"
#include "Core/Util/TextureLoader.h"

#include "Graphics/Platform/Windows/WWindow.h"
#include "Graphics/UI/UIInterface.h"
#include "Graphics/VertexDescription.h"
#include "Graphics/DebugDraw.h"

#include "Samples/ModelViewer.hlsl" // This is a bit nasty... we are forced to include here the file so we can see the declarations.

#include "glm/glm.hpp"

class ModelViewer : public AppBase
{
public:
	void Init();
	void Update()override;
	void Release() override;
private:
	Graphics::BufferHandle m_VertexBuffer;
	Graphics::GraphicsPipeline m_PSO;
	Graphics::BufferHandle m_CB;
	Declarations::ViewerConstants m_Constants;
	int m_NumVertices;
	Graphics::TextureHandle m_TestTexture;
};

ModelViewer app;
ENTRY_POINT(app, "Model Viewer", false);

void ModelViewer::Init()
{
	AppBase::Init();

	m_NumVertices = 0;

	// Load model from file and create vertex buffer
	std::string modelPath = "data:Models/suzanne.obj";
	Core::OBJ::Data modelData = {};
	if (Core::OBJ::LoadFromFile(modelPath, modelData))
	{
		std::vector<VertexData> positionsBuffer;

		for (int i = 0; i < modelData.m_Triangles.size(); ++i)
		{
			const Core::OBJ::Triangle& curTri = modelData.m_Triangles[i];
			for (int j = 0; j < 3; ++j)
			{
				const glm::vec3 curPos = modelData.m_Positions[curTri.m_Positions[j]];
				VertexData curVert;
				curVert.m_Position = glm::vec4(curPos, 1.0f);
				curVert.m_UV = modelData.m_TexCoords[curTri.m_TexCoords[j]];
				positionsBuffer.push_back(curVert);
			}
		}

		m_NumVertices = (int)positionsBuffer.size();
		m_VertexBuffer = m_GraphicsInterface->CreateBuffer(
			Graphics::BufferType::GPUBuffer, Graphics::CPUAccess::None, Graphics::GPUAccess::Read, sizeof(VertexData), m_NumVertices, positionsBuffer.data()
		);
	}

	// Load test texture
	std::string texturePath = "data:Textures/test_grid.jpeg";
	Core::Texture::TextureData loadedTexture;
	if (Core::Texture::LoadFromFile(texturePath, loadedTexture))
	{		
		m_TestTexture =  m_GraphicsInterface->CreateTexture2D(
			loadedTexture.m_Width, loadedTexture.m_Height, 1, 1, 
			loadedTexture.m_Format, 
			Graphics::TextureFlags::TextureFlagNone, 
			loadedTexture.m_Data
		);
	}

	Graphics::GraphicsPipelineDescription psoDesc;
	psoDesc.ColorFormats[0] = m_GraphicsInterface->GetOutputFormat();
	
	psoDesc.VertexShader.ShaderPath = "shadersrc:Samples/ModelViewer.hlsl";
	psoDesc.VertexShader.ShaderEntryPoint = "VSMain";
	psoDesc.VertexShader.Type = Graphics::ShaderType::Vertex;
	
	psoDesc.PixelShader.ShaderPath = "shadersrc:Samples/ModelViewer.hlsl";
	psoDesc.PixelShader.ShaderEntryPoint = "PSMain";
	psoDesc.PixelShader.Type = Graphics::ShaderType::Pixel;

	m_PSO = m_GraphicsInterface->CreateGraphicsPipeline(psoDesc);

	m_CB = m_GraphicsInterface->CreateBuffer(Graphics::BufferType::ConstantBuffer, Graphics::CPUAccess::None, Graphics::GPUAccess::Read, sizeof(m_Constants));	

	// A bit nasty, need to maybe get rid of it.. On graphics creation, we leave the cmdlist open and recording, as we may submit copy commands
	// to init buffers, textures etc..
	// Probably should change the scheme and defer that copy to when we are actually in the "render frame"... Anyways :) 
	m_GraphicsInterface->FlushAndWait();
}

void ModelViewer::Update()
{
	if (ImGui::Begin("Debug textures"))
	{
		ImGui::Image((ImTextureID)m_TestTexture.Handle, ImVec2(128, 128));
		ImGui::End();
	}

	glm::mat4x4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4x4 proj = glm::perspectiveFov(90.0f, (float)m_Window->GetWidth(), (float)m_Window->GetHeight(), 0.1f, 100.0f);

	m_Constants.Model = glm::rotate(glm::mat4x4(), TotalTime, glm::vec3(1.0f, 0.0f, 0.0f));
	m_Constants.Model = glm::rotate(m_Constants.Model, TotalTime, glm::vec3(0.0f, 1.0f, 0.0f));
	m_Constants.Model = glm::rotate(m_Constants.Model, -TotalTime, glm::vec3(0.0f, 0.0f, 1.0f));

	m_Constants.ViewProjection = proj * view;

	m_GraphicsInterface->SetScissor(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

	m_GraphicsInterface->SetConstantBuffer(m_CB, Declarations::kViewerConstantsSlot, sizeof(m_Constants), &m_Constants);

	m_GraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
	m_GraphicsInterface->SetGraphicsPipeline(m_PSO);
	if (m_NumVertices > 0)
	{
		m_GraphicsInterface->SetResource(m_VertexBuffer, Declarations::kg_VertexBufferSlot);
		m_GraphicsInterface->SetResource(m_TestTexture, Declarations::kg_TestTextureSlot);
		m_GraphicsInterface->Draw(m_NumVertices, 0);
	}

	// Debug draw needs re-thinking... Don't want to have to flush manually and pass the camera, maybe pass the camera on item submissions?
	Graphics::DebugDraw::GetInstance()->DrawWireSphere(glm::vec3(0, 4, 5), 2.0f);
	Graphics::DebugDraw::GetInstance()->Flush(m_Constants.ViewProjection);
}

void ModelViewer::Release()
{

}