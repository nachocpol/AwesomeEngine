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
	// GBuffer pipeline
	{
		Graphics::GraphicsPipelineDescription pdesc = {};
		pdesc.PixelShader.ShaderEntryPoint	= "PSGBuffer";
		pdesc.PixelShader.ShaderPath		= "Deferred.hlsl";
		pdesc.PixelShader.Type				= Graphics::Pixel;
		pdesc.VertexShader.ShaderEntryPoint = "VSGBuffer";
		pdesc.VertexShader.ShaderPath		= "Deferred.hlsl";
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
		pdesc.ColorFormats[0]				= Graphics::Format::RGBA_16_Float;
		pdesc.ColorFormats[1]				= Graphics::Format::RGBA_8_Snorm;
		pdesc.ColorFormats[2]				= Graphics::Format::RGBA_16_Float;
		mGBufferPipeline = mGraphics->CreateGraphicsPipeline(pdesc);
	}
	// Light pass pipeline
	{
		Graphics::GraphicsPipelineDescription desc;
		desc.VertexShader.ShaderEntryPoint	= "VSLightPass";
		desc.VertexShader.ShaderPath		= "Deferred.hlsl";
		desc.VertexShader.Type				= Graphics::Vertex;

		desc.PixelShader.ShaderEntryPoint	= "PSLightPass";
		desc.PixelShader.ShaderPath		= "Deferred.hlsl";
		desc.PixelShader.Type				= Graphics::Pixel;

		Graphics::VertexInputDescription::VertexInputElement eles[1] =
		{
			"POSITION",0,Graphics::Format::RGB_32_Float,0
		};
		desc.VertexDescription.Elements		= eles;
		desc.VertexDescription.NumElements	= 1;
		desc.DepthEnabled					= false;
		desc.ColorFormats[0]				= Graphics::Format::RGBA_16_Float;
		{
			desc.BlendTargets[0].Enabled		= true;
			desc.BlendTargets[0].SrcBlendColor	= Graphics::BlendFunction::BlendOne;
			desc.BlendTargets[0].DstBlendColor	= Graphics::BlendFunction::BlendOne;
			desc.BlendTargets[0].BlendOpColor	= Graphics::BlendOperation::BlendOpAdd;
			
			desc.BlendTargets[0].SrcBlendAlpha = Graphics::BlendFunction::BlendOne;
			desc.BlendTargets[0].DstBlendAlpha = Graphics::BlendFunction::BlendOne;
			desc.BlendTargets[0].BlendOpAlpha = Graphics::BlendOperation::BlendOpAdd;
		}
		mLightPassPipeline = mGraphics->CreateGraphicsPipeline(desc);

		mLightInfoHandle = mGraphics->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(mLightInfo));
	}

	// Load test textures
	{
		unsigned char* tData = nullptr;
		int x, y;
		Graphics::Format format;
		if (mAssetImporter->LoadTexture("white.png", tData, x, y, format))
		{
			mWhiteTexture = mGraphics->CreateTexture2D(x, y, 1, 1, format, Graphics::TextureFlagNone, tData);
			mAssetImporter->FreeLoadedTexture(tData);
		}
	}

	// Load models
	//mAssetImporter->LoadModel("mitsuba/mitsuba-sphere.obj", this);
	//mAssetImporter->LoadModel("cube.obj", this);
	mAssetImporter->LoadModel("sponza/sponza.obj", this);

	// Cb
	mAppDataHandle = mGraphics->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(AppData));
	mAppData.View		= glm::lookAt(glm::vec3(10.0f,10.0f,0.0f), glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0));
	mAppData.Projection = glm::perspective(glm::radians(85.0f), 1280.0f / 920.0f, 0.1f, 500.0f);

	mMaterialInfoHandle = mGraphics->CreateBuffer(Graphics::ConstantBuffer, Graphics::None, sizeof(MaterialInfo));

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
		desc.VertexDescription.NumElements	= 1;
		desc.VertexDescription.Elements		= eles;
		desc.ColorFormats[0]				= mGraphics->GetOutputFormat();
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
	float clearBlue[4] = { 0.2f,0.2f,0.3f,1.0f };
	float clearBlack[4] = { 0.0f,0.0f,0.0f,1.0f };

	// GBuffer pass
	{
		Graphics::TextureHandle gbuffer[3] = { mGBuffer.Color,mGBuffer.Normals,mGBuffer.Position };
		mGraphics->SetTargets(3, gbuffer, &mGBuffer.Depth);
		mGraphics->ClearTargets(3, gbuffer, clearBlack, &mGBuffer.Depth, 1.0f, 0);
	
		mGraphics->SetGraphicsPipeline(mGBufferPipeline);
		mGraphics->SetScissor(0.0f, 0.0f, 1280.0f, 920.0f); // dont like this

		for (int i = 0; i < mActors.size(); i++)
		{
			mGraphics->SetTopology(Graphics::TriangleList);
			const auto curActor = mActors[i];
			mAppData.Model = glm::mat4(1.0f);
			mAppData.Model = glm::scale(mAppData.Model, glm::vec3(0.1f));
			//mAppData.Model = glm::rotate(mAppData.Model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			mGraphics->SetConstantBuffer(mAppDataHandle, 0, sizeof(mAppData), &mAppData);

			mGraphics->SetVertexBuffer(curActor->AMesh.VertexBuffer, curActor->AMesh.NumVertex * curActor->AMesh.ElementSize, curActor->AMesh.ElementSize);
			// Albedo texture
			if (CHECK_TEXTURE(curActor->ShadeInfo.AlbedoTexture))
			{
				mGraphics->SetTexture(curActor->ShadeInfo.AlbedoTexture, 0);
			}
			else
			{
				mGraphics->SetTexture(mWhiteTexture, 0);
			}
			// Bump texture
			if (CHECK_TEXTURE(curActor->ShadeInfo.BumpMapTexture))
			{
				mGraphics->SetTexture(curActor->ShadeInfo.BumpMapTexture, 1);
				mMaterialInfo.UseBumpTex = 1;
			}
			else
			{
				mMaterialInfo.UseBumpTex = 0;
			}
			mMaterialInfo.AlbedoColor = curActor->ShadeInfo.AlbedoColor;
			mGraphics->SetConstantBuffer(mMaterialInfoHandle, 1, sizeof(mMaterialInfo), &mMaterialInfo);

			mGraphics->Draw(curActor->AMesh.NumVertex, 0);
		}

		mGraphics->DisableAllTargets();
	}

	// Light pass
	{
		mGraphics->SetTargets(1, &mLightPass, nullptr);
		mGraphics->ClearTargets(1, &mLightPass, clearBlack, nullptr, 0.0f, 0);
		mGraphics->SetGraphicsPipeline(mLightPassPipeline);
		mGraphics->SetVertexBuffer(mFullScreenQuad, sizeof(VertexScreen) * 6, sizeof(VertexScreen));
		{
			mGraphics->SetTexture(mGBuffer.Color, 0);
			mGraphics->SetTexture(mGBuffer.Normals, 1);

			mLightInfo.LightColor = glm::vec4(1.0f, 1.0f, 1.0f,1.0f);
			mLightInfo.LightPosition = glm::vec4(-0.5f,-0.5f,-1.0f,0.0f);
			mGraphics->SetConstantBuffer(mLightInfoHandle, 1, sizeof(mLightInfo), &mLightInfo);
			mGraphics->Draw(6, 0);

			// mGraphics->SetTexture(mGBuffer.Color, 0);
			// mGraphics->SetTexture(mGBuffer.Normals, 1);
			// 
			// mLightInfo.LightColor = glm::vec4(0.8f, 0.3f, 0.3f, 1.0f);
			// mLightInfo.LightPosition = glm::vec4( 0.5f,  -0.25f, -1.0f, 0.0f);
			// mGraphics->SetConstantBuffer(mLightInfoHandle, 1, sizeof(mLightInfo), &mLightInfo);
			// mGraphics->Draw(6, 0);
		}
		mGraphics->DisableAllTargets();
	}

	// Post processing and display
	mGraphics->SetGraphicsPipeline(mFullScreenPipeline);
	mGraphics->SetTexture(mLightPass, 0);
	mGraphics->SetVertexBuffer(mFullScreenQuad, sizeof(VertexScreen) * 6, sizeof(VertexScreen));
	mGraphics->Draw(6, 0);
}

void ShowcaseScene::Resize(int w, int h)
{
	mGBuffer.Color = mGraphics->CreateTexture2D(w, h, 1, 1, Graphics::Format::RGBA_16_Float, Graphics::TextureFlags::RenderTarget);
	mGBuffer.Normals = mGraphics->CreateTexture2D(w, h, 1, 1, Graphics::Format::RGBA_8_Snorm, Graphics::TextureFlags::RenderTarget);
	mGBuffer.Position = mGraphics->CreateTexture2D(w, h, 1, 1, Graphics::Format::RGBA_16_Float, Graphics::TextureFlags::RenderTarget);
	mGBuffer.Depth = mGraphics->CreateTexture2D(w, h, 1, 1, Graphics::Format::Depth24_Stencil8, Graphics::TextureFlags::DepthStencil);

	mLightPass = mGraphics->CreateTexture2D(w, h, 1, 1, Graphics::Format::RGBA_16_Float, Graphics::TextureFlags::RenderTarget);
}
