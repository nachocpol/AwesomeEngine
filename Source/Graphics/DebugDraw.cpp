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
	mCameraDataCb = mGraphicsInterface->CreateBuffer(BufferType::ConstantBuffer, CPUAccess::None, GPUAccess::Read, sizeof(CameraData));
	mItemDataCb = mGraphicsInterface->CreateBuffer(BufferType::ConstantBuffer, CPUAccess::None, GPUAccess::Read, sizeof(ItemData));

	// Init vertex buffers
	mLinesVtxBuffer = mGraphicsInterface->CreateBuffer(BufferType::VertexBuffer, CPUAccess::Write, GPUAccess::Read, MAX_LINES * 2 * sizeof(DebugVertex));

	// Wire sphere:
	{
		uint32_t latitudeSegments = 32;
		uint32_t longitudeSegments = 32;
		float radius = 1.0f;

		std::vector<DebugVertex> sphereVtx;
		sphereVtx.resize(latitudeSegments * 2 + longitudeSegments * 2);

		// Latitude:
		{
			float angleDelta = glm::two_pi<float>() / (float)latitudeSegments;
			float curAngle = 0.0f;
			for (uint32_t segment = 0; segment < latitudeSegments * 2; segment += 2)
			{
				DebugVertex vtx;
				float x = glm::sin(curAngle * radius);
				float z = glm::cos(curAngle * radius);
				vtx.Position = glm::vec3(x, 0.0f, z);
				vtx.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				sphereVtx[segment] = vtx;

				x = glm::sin((curAngle + angleDelta) * radius);
				z = glm::cos((curAngle + angleDelta) * radius);
				vtx.Position = glm::vec3(x, 0.0f, z);
				vtx.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				sphereVtx[segment + 1] = vtx;

				curAngle += angleDelta;
			}
		}
		uint32_t vtxOff = latitudeSegments * 2;
		// Longitude:
		{
			float angleDelta = glm::two_pi<float>() / (float)latitudeSegments;
			float curAngle = 0.0f;
			for (uint32_t segment = 0; segment < longitudeSegments * 2; segment += 2)
			{
				DebugVertex vtx;
				float x = glm::sin(curAngle * radius);
				float y = glm::cos(curAngle * radius);
				vtx.Position = glm::vec3(x, y, 0.0f);
				vtx.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				sphereVtx[vtxOff + segment] = vtx;

				x = glm::sin((curAngle + angleDelta) * radius);
				y = glm::cos((curAngle + angleDelta) * radius);
				vtx.Position = glm::vec3(x, y, 0.0f);
				vtx.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				sphereVtx[vtxOff + segment + 1] = vtx;

				curAngle += angleDelta;
			}
		}

		mWireSphereNumVtx = (uint32_t)sphereVtx.size();
		mWireSphereVtxBuffer = mGraphicsInterface->CreateBuffer(BufferType::VertexBuffer, CPUAccess::None, GPUAccess::Read, sizeof(DebugVertex) * mWireSphereNumVtx, 0, &sphereVtx[0]);
	}
}

void DebugDraw::Release()
{
}

void Graphics::DebugDraw::StartFrame()
{
	// Reset items:
	mLines.clear();
	mWireSpheres.clear();
}

void Graphics::DebugDraw::Flush(World::Camera* camera)
{
	// Lines
	if (!mLines.empty())
	{
		uint32_t numLines = (uint32_t)mLines.size();
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
			mItemData.World = glm::mat4(1.0f);
			mItemData.DebugColor = glm::vec4(1.0f);
			mGraphicsInterface->SetGraphicsPipeline(mDebugPipeline);
			mGraphicsInterface->SetConstantBuffer(mCameraDataCb, kCameraDataSlot, sizeof(CameraData), &mCameraData);
			mGraphicsInterface->SetConstantBuffer(mItemDataCb, kItemDataSlot, sizeof(ItemData), &mItemData);
			mGraphicsInterface->SetTopology(Topology::LineList);
			mGraphicsInterface->SetVertexBuffer(mLinesVtxBuffer, sizeof(DebugVertex) * 2 * numLines, sizeof(DebugVertex));
			mGraphicsInterface->Draw(numLines * 2, 0);
		}
	}

	// Wire spheres
	if (!mWireSpheres.empty())
	{
		for (const auto sphere : mWireSpheres)
		{
			mItemData.World = glm::mat4(1.0f);
			mItemData.World = glm::translate(mItemData.World, sphere.Center);
			mItemData.World = glm::scale(mItemData.World, glm::vec3(sphere.Radius));

			mItemData.DebugColor = sphere.Color;

			mGraphicsInterface->SetGraphicsPipeline(mDebugPipeline);
			mGraphicsInterface->SetConstantBuffer(mCameraDataCb, kCameraDataSlot, sizeof(CameraData), &mCameraData);
			mGraphicsInterface->SetConstantBuffer(mItemDataCb, kItemDataSlot, sizeof(ItemData), &mItemData);
			mGraphicsInterface->SetTopology(Topology::LineList);
			mGraphicsInterface->SetVertexBuffer(mWireSphereVtxBuffer, sizeof(DebugVertex) * mWireSphereNumVtx, sizeof(DebugVertex));
			mGraphicsInterface->Draw(mWireSphereNumVtx, 0);
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

void Graphics::DebugDraw::DrawWireSphere(glm::vec3 center, float radius, glm::vec4 color /*== glm::vec4(1.0f)*/)
{
	if (mWireSpheres.size() >= MAX_WIRE_SPHERES)
	{
		ERR("Reached maximum number of wire spheres");
		return;
	}
	mWireSpheres.push_back(WireSphereItem(center, radius, color));
}

void Graphics::DebugDraw::DrawFrustum(glm::mat4 transform, float aspect, float vfov, float nearDist, float farDist, glm::vec4 color /*= glm::vec4(1.0f)*/)
{
	float halfVFov = glm::radians(vfov * 0.5f);
	float tanHalfVFov = glm::tan(halfVFov);

	// Near plane
	float halfHeightNear = tanHalfVFov * nearDist;
	float halfWidthNear = aspect * halfHeightNear;
	glm::vec3 TRNear = transform * glm::vec4( halfWidthNear,  halfHeightNear, nearDist, 1.0f);
	glm::vec3 BRNear = transform * glm::vec4( halfWidthNear, -halfHeightNear, nearDist, 1.0f);
	glm::vec3 BLNear = transform * glm::vec4(-halfWidthNear, -halfHeightNear, nearDist, 1.0f);
	glm::vec3 TLNear = transform * glm::vec4(-halfWidthNear,  halfHeightNear, nearDist, 1.0f);
	DrawLine(TRNear, BRNear, color);
	DrawLine(BRNear, BLNear, color);
	DrawLine(BLNear, TLNear, color);
	DrawLine(TLNear, TRNear, color);

	// Far plane
	float halfHeightFar = tanHalfVFov * farDist;
	float halfWidthFar = aspect * halfHeightFar;
	glm::vec3 TRFar = transform * glm::vec4( halfWidthFar,  halfHeightFar, farDist, 1.0f);
	glm::vec3 BRFar = transform * glm::vec4( halfWidthFar, -halfHeightFar, farDist, 1.0f);
	glm::vec3 BLFar = transform * glm::vec4(-halfWidthFar, -halfHeightFar, farDist, 1.0f);
	glm::vec3 TLFar = transform * glm::vec4(-halfWidthFar,  halfHeightFar, farDist, 1.0f);
	DrawLine(TRFar, BRFar, color);
	DrawLine(BRFar, BLFar, color);
	DrawLine(BLFar, TLFar, color);
	DrawLine(TLFar, TRFar, color);

	// Sides
	DrawLine(TRFar, TRNear, color);
	DrawLine(TLFar, TLNear, color);
	DrawLine(BRFar, BRNear, color);
	DrawLine(BLFar, BLNear, color);
}

DebugDraw::LineItem::LineItem(glm::vec3 start, glm::vec3 end, glm::vec4 color):
	 Start(start)
	,End(end)
	,Color(color)
{
}

DebugDraw::WireSphereItem::WireSphereItem(glm::vec3 center,float radius, glm::vec4 color):
	 Center(center)
	,Radius(radius)
	,Color(color)
{
}
