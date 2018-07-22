#include "Graphics/DX12/DX12GraphicsInterface.h"
#include "Graphics/Platform/Windows/WWindow.h"
#include "Graphics/AssetImporter.h"
#include <stdio.h>

Graphics::GraphicsInterface* gGraphicsInterface = nullptr;
Graphics::AssetImporter* gImporter = nullptr;

struct Vertex
{
	float x, y, z;
	float r, g, b;
	float u, v;
};
struct DrawData
{
	float Time;
}DrawData;
struct OtherData
{
	float x,y,z;
}OtherData;
Graphics::BufferHandle vertexBuffer;
Graphics::GraphicsPipeline pipeline;
Graphics::BufferHandle drawDataBuffer;
Graphics::BufferHandle otherDataBuffer;
Graphics::TextureHandle proceduralTex;

bool InitGraphics(Graphics::Platform::BaseWindow* window);
void InitResources();

int main()
{
	wchar_t wdir[128];
	GetModuleFileName(NULL, wdir, 128);

	auto window = new Graphics::Platform::Windows::WWindow();
	window->Initialize("Awesome Advanced", false, 1280, 920);
	
	InitGraphics(window);
	gImporter = new Graphics::AssetImporter(gGraphicsInterface);
	gImporter->Load("mitsuba\\mitsuba.obj");
	InitResources();
	
	
	gGraphicsInterface->FlushAndWait();

	bool running = true;
	while (running)
	{
		static float t = 0.0f;
		t += 0.16f;

		window->Update();

		gGraphicsInterface->StartFrame();
		gGraphicsInterface->SetScissor(0.0f, 0.0f, window->GetWidth(), window->GetHeight());
		gGraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
		gGraphicsInterface->SetGraphicsPipeline(pipeline);
		gGraphicsInterface->SetTexture(proceduralTex, 0);
		DrawData.Time = t * 3.0f;
		gGraphicsInterface->SetConstantBuffer(drawDataBuffer, 0, sizeof(DrawData), &DrawData);
		OtherData.x = sin(t * 0.12f) * 0.5f;
		gGraphicsInterface->SetConstantBuffer(otherDataBuffer, 1, sizeof(OtherData), &OtherData);
		gGraphicsInterface->SetVertexBuffer(vertexBuffer, sizeof(Vertex) * 6, sizeof(Vertex));
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
	Vertex arr[6] =
	{
		-0.5f, 0.5f,0.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,
		 0.5f, 0.5f,0.0f, 0.0f,0.0f,0.0f, 1.0f,0.0f,
		 0.5f,-0.5f,0.0f, 0.0f,0.0f,0.0f, 1.0f,1.0f,

		-0.5f, 0.5f,0.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,
		 0.5f,-0.5f,0.0f, 0.0f,0.0f,0.0f, 1.0f,1.0f,
		-0.5f,-0.5f,0.0f, 0.0f,0.0f,0.0f, 0.0f,1.0f,
	};
	vertexBuffer = gGraphicsInterface->CreateBuffer(Graphics::VertexBuffer, Graphics::CPUAccess::None, sizeof(Vertex) * 6, &arr[0]);
	{
		Graphics::GraphicsPipelineDescription pdesc = {};
		pdesc.PixelShader.ShaderEntryPoint = "PSSimple";
		pdesc.PixelShader.ShaderPath = "AdvancedSample.hlsl";
		pdesc.PixelShader.Type = Graphics::Pixel;

		pdesc.VertexShader.ShaderEntryPoint = "VSSimple";
		pdesc.VertexShader.ShaderPath = "AdvancedSample.hlsl";
		pdesc.VertexShader.Type = Graphics::Vertex;

		Graphics::VertexInputDescription::VertexInputElement eles[3];
		eles[0].Semantic = "POSITION";
		eles[0].Idx = 0;
		eles[0].EleFormat = Graphics::Format::RGB_32_Float;
		eles[0].Offset = 0;

		eles[1].Semantic = "COLOR";
		eles[1].Idx = 0;
		eles[1].EleFormat = Graphics::Format::RGB_32_Float;
		eles[1].Offset = sizeof(float) * 3;

		eles[2].Semantic = "UV";
		eles[2].Idx = 0;
		eles[2].EleFormat = Graphics::Format::RG_32_Float;
		eles[2].Offset = sizeof(float) * 3 * 2;

		pdesc.VertexDescription.NumElements = sizeof(eles) / sizeof(Graphics::VertexInputDescription::VertexInputElement);
		pdesc.VertexDescription.Elements = eles;

		pipeline = gGraphicsInterface->CreateGraphicsPipeline(pdesc);
	}
	drawDataBuffer = gGraphicsInterface->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(DrawData));
	otherDataBuffer = gGraphicsInterface->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(OtherData));
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
		proceduralTex = gGraphicsInterface->CreateTexture2D(w, h, 1, 1, Graphics::Format::RGBA_32_Float, texels);
	}
}

