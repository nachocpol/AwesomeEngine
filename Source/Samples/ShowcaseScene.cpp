#include "ShowcaseScene.h"
#include <stdint.h>

struct VertexScreen
{
	float x, y, z;
};

ShowcaseScene::ShowcaseScene(Graphics::GraphicsInterface* graphics, Graphics::AssetImporter* assetImp):
	Scene::Scene(graphics,assetImp)
{
}

ShowcaseScene::~ShowcaseScene()
{
}

bool ShowcaseScene::Initialize()
{
	Graphics::GraphicsPipelineDescription pdesc = {};
	pdesc.PixelShader.ShaderEntryPoint	= "PSFordwardSimple";
	pdesc.PixelShader.ShaderPath		= "Fordward.hlsl";
	pdesc.PixelShader.Type				= Graphics::Pixel;

	pdesc.VertexShader.ShaderEntryPoint = "VSFordwardSimple";
	pdesc.VertexShader.ShaderPath		= "Fordward.hlsl";
	pdesc.VertexShader.Type				= Graphics::Vertex;

	const size_t v3Size = sizeof(glm::vec3);
	Graphics::VertexInputDescription::VertexInputElement eles[4] =
	{
		{ "POSITION",0,Graphics::Format::RGB_32_Float,0 },
		{ "NORMAL",0,Graphics::Format::RGB_32_Float,v3Size },
		{ "TANGENT",0,Graphics::Format::RGB_32_Float,v3Size * 2 },
		{ "TEXCOORD",0,Graphics::Format::RG_32_Float,v3Size * 3 },
	};
	pdesc.VertexDescription.Elements	= eles;
	pdesc.VertexDescription.NumElements = 4;

	pdesc.DepthEnabled					= true;
	pdesc.DepthFunction					= Graphics::LessEqual;
	pdesc.DepthFormat					= Graphics::Depth24_Stencil8;
	mFordwardPipeline = mGraphics->CreateGraphicsPipeline(pdesc);

	// Load test textures
	{
		unsigned char* tData = nullptr;
		int x, y;
		Graphics::Format format;
		if (mAssetImporter->LoadTexture("tiles05/Tiles05_COL_VAR1_1K.jpg", tData, x, y, format))
		{
			mTestAlbedo = mGraphics->CreateTexture2D(x, y, 1, 1, format, Graphics::TextureFlagNone, tData);
			mAssetImporter->FreeLoadedTexture(tData);
		}
	}
	{
		unsigned char* tData = nullptr;
		int x, y;
		Graphics::Format format;
		if (mAssetImporter->LoadTexture("tiles05/Tiles05_NRM_1K.jpg", tData, x, y, format))
		{
			mTestBump = mGraphics->CreateTexture2D(x, y, 1, 1, format, Graphics::TextureFlagNone, tData);
			mAssetImporter->FreeLoadedTexture(tData);
		}
	}

	// Load models
	Graphics::Mesh* loadedMeshes = nullptr;
	uint8_t numLoadedMeshes = 0;
	//mAssetImporter->LoadModel("mitsuba/mitsuba-sphere.obj", loadedMeshes, numLoadedMeshes);
	mAssetImporter->LoadModel("cube.obj", loadedMeshes, numLoadedMeshes);
	
	for (int i = 0; i < numLoadedMeshes; i++)
	{
		auto actor = AddActor();
		actor->AMesh = loadedMeshes[i];
		actor->ShadeInfo.AlbedoTexture = mTestAlbedo;
		actor->ShadeInfo.BumpMapTexture = mTestBump;
	}

	// Cb
	mAppDataHandle = mGraphics->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(AppData));
	mAppData.View		= glm::lookAt(glm::vec3(0.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0));
	mAppData.Projection = glm::perspective(glm::radians(85.0f), 1280.0f / 920.0f, 0.1f, 100.0f);

	// Full screen stuf
	{
		Graphics::GraphicsPipelineDescription desc;
		desc.DepthEnabled					= false;
		desc.DepthFunction					= Graphics::Always;
		desc.VertexShader.ShaderEntryPoint	= "VSFullScreen";
		desc.VertexShader.ShaderPath		= "Common.hlsl";
		desc.VertexShader.Type				= Graphics::Vertex;
		desc.PixelShader.ShaderEntryPoint	= "PSFullScreen";
		desc.PixelShader.ShaderPath			= "Common.hlsl";
		desc.PixelShader.Type				= Graphics::Pixel;
		Graphics::VertexInputDescription::VertexInputElement eles[1] =
		{
			"POSITION",0, Graphics::Format::RGB_32_Float,0
		};
		desc.VertexDescription.NumElements	 = 1;
		desc.VertexDescription.Elements		= eles;
		mFullScreenPipeline					= mGraphics->CreateGraphicsPipeline(desc);
		VertexScreen vtxData[6] =
		{
			-1.0f, 1.0f,0.0f,
			1.0f, 1.0f,0.0f,
			1.0f,-1.0f,0.0f,
			-1.0f, 1.0f,0.0f,
			1.0f,-1.0f,0.0f,
			-1.0f,-1.0f,0.0f
		};
		mFullScreenQuad = mGraphics->CreateBuffer(Graphics::VertexBuffer, Graphics::None, sizeof(VertexScreen) * 6, &vtxData);
	}

	return true;
}

void ShowcaseScene::Update(float dt)
{
}

void ShowcaseScene::Draw(float dt)
{
	mGraphics->SetTargets(1, &mMainTarget, &mMainDepthTarget);
	float clear[4] = { 0.2f,0.2f,0.3f,1.0f };
	mGraphics->ClearTargets(1, &mMainTarget, clear, &mMainDepthTarget, 1.0f, 0);
	mGraphics->SetTopology(Graphics::Topology::TriangleList);
	mGraphics->SetScissor(0.0f, 0.0f, 1280.0f, 920.0f);
	mGraphics->SetGraphicsPipeline(mFordwardPipeline);
	for (int i = 0; i < mActors.size(); i++)
	{
		const auto curActor = mActors[i];
		mAppData.Model = glm::mat4(1.0f);
		mAppData.Model = glm::translate(mAppData.Model, glm::vec3(0.0f, 0.0f, 0.0f));
		mAppData.Model = glm::scale(mAppData.Model, glm::vec3(1.0f));
		mAppData.DebugColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		mGraphics->SetVertexBuffer(curActor->AMesh.VertexBuffer, curActor->AMesh.ElementSize * curActor->AMesh.NumVertex, curActor->AMesh.ElementSize);
		mGraphics->SetConstantBuffer(mAppDataHandle, 0, sizeof(mAppData), &mAppData);
		{
			if (CHECK_TEXTURE(curActor->ShadeInfo.AlbedoTexture))
			{
				mGraphics->SetTexture(curActor->ShadeInfo.AlbedoTexture,0);
			}
			if (CHECK_TEXTURE(curActor->ShadeInfo.BumpMapTexture))
			{
				mGraphics->SetTexture(curActor->ShadeInfo.BumpMapTexture, 1);
			}
		}
		mGraphics->Draw(curActor->AMesh.NumVertex, 0);
	}
	mGraphics->DisableAllTargets();

	mGraphics->SetGraphicsPipeline(mFullScreenPipeline);
	mGraphics->SetTexture(mMainTarget, 0);
	mGraphics->SetVertexBuffer(mFullScreenQuad, sizeof(VertexScreen) * 6, sizeof(VertexScreen));
	mGraphics->Draw(6, 0);
}

void ShowcaseScene::Resize(int w, int h)
{
	mMainTarget = mGraphics->CreateTexture2D(w, h, 1, 1, Graphics::Format::RGBA_8_Unorm, Graphics::TextureFlags::RenderTarget);
	mMainDepthTarget = mGraphics->CreateTexture2D(w, h, 1, 1, Graphics::Format::Depth24_Stencil8, Graphics::TextureFlags::DepthStencil);
}
