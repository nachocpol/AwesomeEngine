#include "DebugDraw.h"
#include "World/Camera.h"
#include "Core/Logging.h"

using namespace Graphics;

struct DebugVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
};

DebugDraw::DebugDraw()
{
}

DebugDraw::~DebugDraw()
{
}

DebugDraw* Graphics::DebugDraw::GetInstance()
{
	static DebugDraw* kInstance = nullptr;
	if (!kInstance)
	{
		kInstance = new DebugDraw;
	}
	return kInstance;
}

void DebugDraw::Initialize(GraphicsInterface* graphicsInterface)
{
	mGraphicsInterface = graphicsInterface;

	GraphicsPipelineDescription pdesc = {};
	pdesc.PixelShader.ShaderEntryPoint = "PSDebugDraw";
	pdesc.PixelShader.ShaderPath = "DebugDraw.hlsl";
	pdesc.PixelShader.Type = Graphics::ShaderType::Pixel;

	pdesc.VertexShader.ShaderEntryPoint = "VSDebugDraw";
	pdesc.VertexShader.ShaderPath = "DebugDraw.hlsl";
	pdesc.VertexShader.Type = Graphics::ShaderType::Vertex;

	Graphics::VertexInputDescription::VertexInputElement eles[2] =
	{
		{ "POSITION", 0, Graphics::Format::RGB_32_Float,  0  },
		{ "COLOR",    0, Graphics::Format::RGBA_32_Float, 12 }
	};

	pdesc.PrimitiveType = Primitive::Line;
	pdesc.VertexDescription.NumElements = sizeof(eles) / sizeof(Graphics::VertexInputDescription::VertexInputElement);
	pdesc.VertexDescription.Elements = eles;
	pdesc.DepthEnabled = true;
	pdesc.DepthWriteEnabled = false;
	pdesc.DepthFunction = Graphics::LessEqual;
	pdesc.ColorFormats[0] = Graphics::Format::RGBA_8_Unorm;
	pdesc.DepthFormat = Graphics::Format::Depth24_Stencil8;

	mDebugPipeline = mGraphicsInterface->CreateGraphicsPipeline(pdesc);

	// Init constant buffers:
	mCameraDataCb = mGraphicsInterface->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(CameraData));

	// Init vertex buffers
	mLinesVtxBuffer = mGraphicsInterface->CreateBuffer(Graphics::VertexBuffer, CPUAccess::Write, MAX_LINES * 2 * sizeof(DebugVertex));
}

void DebugDraw::Release()
{
}

void Graphics::DebugDraw::StartFrame()
{
	// Reset items:
	mLines.clear();
}

void Graphics::DebugDraw::Flush(World::Camera* camera)
{
	// Lines
	if (!mLines.empty())
	{
		uint32_t numLines = mLines.size();
		unsigned char* pLines = nullptr;
		if (mGraphicsInterface->MapBuffer(mLinesVtxBuffer, &pLines))
		{
			DebugVertex* pVtx = (DebugVertex*)pLines;
			for (const auto line : mLines)
			{
				pVtx->Position = line.Start;
				pVtx->Color = line.Color;
				++pVtx;

				pVtx->Position = line.End;
				pVtx->Color = line.Color;
				++pVtx;
			}
			mGraphicsInterface->UnMapBuffer(mLinesVtxBuffer);

			mCameraData.InvViewProj = camera->GetProjection() * camera->GetInvViewTransform();
			mGraphicsInterface->SetGraphicsPipeline(mDebugPipeline);
			mGraphicsInterface->SetConstantBuffer(mCameraDataCb, 0, sizeof(CameraData), &mCameraData);
			mGraphicsInterface->SetTopology(Topology::LineList);
			mGraphicsInterface->SetVertexBuffer(mLinesVtxBuffer, sizeof(DebugVertex) * 2 * numLines, sizeof(DebugVertex));
			mGraphicsInterface->Draw(numLines * 2, 0);
		}
	}

	mGraphicsInterface->SetTopology(Topology::TriangleList);
}

void Graphics::DebugDraw::EndFrame()
{
}

void DebugDraw::DrawLine(glm::vec3 start, glm::vec3 end, glm::vec4 color /*= glm::vec4(1.0f)*/)
{
	if (mLines.size() >= MAX_LINES)
	{
		ERR("Reached maximum number of lines");
		return;
	}
	mLines.push_back(LineItem(start, end, color));
}

void Graphics::DebugDraw::DrawAABB(glm::vec3 min, glm::vec3 max, glm::vec4 color /*= glm::vec4(1.0f)*/)
{
	glm::vec3 FrontTR = max;
	glm::vec3 FrontTL = glm::vec3(min.x, max.y, max.z);

	glm::vec3 BackTR = glm::vec3(max.x, max.y, min.z);
	glm::vec3 BackTL = glm::vec3(min.x, max.y, min.z);

	glm::vec3 FrontBR = glm::vec3(max.x, min.y, max.z);
	glm::vec3 FrontBL = glm::vec3(min.x, min.y, max.z);

	glm::vec3 BackBR = glm::vec3(max.x, min.y, min.z);
	glm::vec3 BackBL = glm::vec3(min.x, min.y, min.z);

	// Top!
	DrawLine(FrontTR, FrontTL, color);
	DrawLine(FrontTL, BackTL, color);
	DrawLine(BackTL, BackTR, color);
	DrawLine(BackTR, FrontTR, color);

	// Bottom
	DrawLine(FrontBR, FrontBL, color);
	DrawLine(FrontBL, BackBL, color);
	DrawLine(BackBL, BackBR, color);
	DrawLine(BackBR, FrontBR, color);

	// Sides
	DrawLine(FrontTR, FrontBR, color);
	DrawLine(FrontTL, FrontBL, color);
	DrawLine(BackTR, BackBR, color);
	DrawLine(BackTL, BackBL, color);
}

DebugDraw::LineItem::LineItem(glm::vec3 start, glm::vec3 end, glm::vec4 color):
	 Start(start)
	,End(end)
	,Color(color)
{
}
