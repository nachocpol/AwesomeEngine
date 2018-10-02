#include "Graphics/DX12/DX12GraphicsInterface.h"
#include "Graphics/Platform/Windows/WWindow.h"
#include <stdio.h>

Graphics::GraphicsInterface* gGraphicsInterface = nullptr;
struct Vertex
{
	float x, y, z;
	float r, g, b;
};
Graphics::BufferHandle vertexBuffer;
Graphics::GraphicsPipeline pipeline;

bool InitGraphics(Graphics::Platform::BaseWindow* window);
void InitResources();

int main()
{
	auto window = new Graphics::Platform::Windows::WWindow();
	window->Initialize("Awesome Triangle", false, 1280, 920);
	
	InitGraphics(window);
	InitResources();
	gGraphicsInterface->FlushAndWait();

	bool running = true;
	while (running)
	{
		window->Update();

		gGraphicsInterface->StartFrame();
		gGraphicsInterface->SetScissor(0.0f, 0.0f, (float)window->GetWidth(), (float)window->GetHeight());
		gGraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
		gGraphicsInterface->SetGraphicsPipeline(pipeline);
		gGraphicsInterface->SetVertexBuffer(vertexBuffer, sizeof(Vertex) * 3, sizeof(Vertex));
		gGraphicsInterface->Draw(3, 0);
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
	Vertex arr[3] =
	{
		-0.5f, -0.5f, 0.5f,   1.0f, 0.0f, 0.0f,
		 0.0f,  0.5f, 0.5f,   0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 1.0f
	};
	vertexBuffer = gGraphicsInterface->CreateBuffer(Graphics::VertexBuffer, Graphics::CPUAccess::None, sizeof(Vertex) * 3, &arr[0]);
	{
		Graphics::GraphicsPipelineDescription pdesc = {};
		pdesc.PixelShader.ShaderEntryPoint = "PSSimple";
		pdesc.PixelShader.ShaderPath = "TriangleSample.hlsl";
		pdesc.PixelShader.Type = Graphics::ShaderType::Pixel;

		pdesc.VertexShader.ShaderEntryPoint = "VSSimple";
		pdesc.VertexShader.ShaderPath = "TriangleSample.hlsl";
		pdesc.VertexShader.Type = Graphics::ShaderType::Vertex;

		Graphics::VertexInputDescription::VertexInputElement eles[2];
		eles[0].Semantic = "POSITION";
		eles[0].Idx = 0;
		eles[0].EleFormat = Graphics::Format::RGB_32_Float;
		eles[0].Offset = 0;

		eles[1].Semantic = "COLOR";
		eles[1].Idx = 0;
		eles[1].EleFormat = Graphics::Format::RGB_32_Float;
		eles[1].Offset = sizeof(float) * 3;

		pdesc.VertexDescription.NumElements = sizeof(eles) / sizeof(Graphics::VertexInputDescription::VertexInputElement);
		pdesc.VertexDescription.Elements = eles;

		pipeline = gGraphicsInterface->CreateGraphicsPipeline(pdesc);
	}
}

