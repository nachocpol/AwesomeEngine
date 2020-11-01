#include "CloudRendering.h"
#include "Graphics/UI/IMGUI/imgui.h"
#include "Graphics/Noise.h"
#include "Graphics/Profiler.h"
#include "Graphics/VertexDescription.h"

#include "glm/gtc/packing.hpp"

namespace Graphics
{
	CloudRenderer::CloudRenderer():
		mGraphicsInterface(nullptr)
	{
	}

	CloudRenderer::~CloudRenderer()
	{
	}

	bool CloudRenderer::Initialize(GraphicsInterface* graphicsInterface)
	{
		mGraphicsInterface = graphicsInterface;
		
		CreateTextures();

		// Render clouds PS
		{
			Graphics::GraphicsPipelineDescription desc;
			desc.DepthEnabled = true;
			desc.DepthWriteEnabled = false;
			desc.DepthFunction = DepthFunc::Equal; // we paint it at far (so we can discard fragments from it)
			desc.DepthFormat = Graphics::Format::Depth24_Stencil8;
			desc.VertexShader.ShaderEntryPoint = "VSClouds";
			desc.VertexShader.ShaderPath = "Clouds.hlsl";
			desc.VertexShader.Type = Graphics::ShaderType::Vertex;
			desc.PixelShader.ShaderEntryPoint = "PSClouds";
			desc.PixelShader.ShaderPath = "Clouds.hlsl";
			desc.PixelShader.Type = Graphics::ShaderType::Pixel;
			desc.VertexDescription = PosVertexDescription::GetDescription(); // TO-DO: test this
			desc.ColorFormats[0] = Graphics::Format::RGBA_16_Float;

			desc.BlendTargets[0].Enabled = true;
			desc.BlendTargets[0].SrcBlendColor = Graphics::BlendFunction::BlendSrcAlpha;
			desc.BlendTargets[0].DstBlendColor = Graphics::BlendFunction::BlendInvSrcAlpha;
			desc.BlendTargets[0].BlendOpColor = Graphics::BlendOperation::BlendOpAdd;

			desc.BlendTargets[0].SrcBlendAlpha = Graphics::BlendFunction::BlendOne;
			desc.BlendTargets[0].DstBlendAlpha = Graphics::BlendFunction::BlendOne;
			desc.BlendTargets[0].BlendOpAlpha = Graphics::BlendOperation::BlendOpAdd;

			mCloudsPipeline = mGraphicsInterface->CreateGraphicsPipeline(desc);
		}

		// Composite clouds PS
		{
			Graphics::GraphicsPipelineDescription desc;
			desc.DepthEnabled = true;
			desc.DepthWriteEnabled = false;
			desc.DepthFunction = DepthFunc::Equal; // we paint it at far (so we can discard fragments from it)
			desc.DepthFormat = Graphics::Format::Depth24_Stencil8;
			desc.VertexShader.ShaderEntryPoint = "VSClouds";
			desc.VertexShader.ShaderPath = "Clouds.hlsl";
			desc.VertexShader.Type = Graphics::ShaderType::Vertex;
			desc.PixelShader.ShaderEntryPoint = "PSCloudsComposite";
			desc.PixelShader.ShaderPath = "Clouds.hlsl";
			desc.PixelShader.Type = Graphics::ShaderType::Pixel;
			desc.VertexDescription = PosVertexDescription::GetDescription(); // TO-DO: test this
			desc.ColorFormats[0] = mGraphicsInterface->GetOutputFormat();

			desc.BlendTargets[0].Enabled = true;
			desc.BlendTargets[0].SrcBlendColor = Graphics::BlendFunction::BlendSrcAlpha;
			desc.BlendTargets[0].DstBlendColor = Graphics::BlendFunction::BlendInvSrcAlpha;
			desc.BlendTargets[0].BlendOpColor = Graphics::BlendOperation::BlendOpAdd;

			desc.BlendTargets[0].SrcBlendAlpha = Graphics::BlendFunction::BlendOne;
			desc.BlendTargets[0].DstBlendAlpha = Graphics::BlendFunction::BlendOne;
			desc.BlendTargets[0].BlendOpAlpha = Graphics::BlendOperation::BlendOpAdd;

			mCloudsCompositePipeline = mGraphicsInterface->CreateGraphicsPipeline(desc);
		}

		mCloudsDataHandle = mGraphicsInterface->CreateBuffer(Graphics::BufferType::ConstantBuffer, Graphics::CPUAccess::None, Graphics::GPUAccess::Read, sizeof(mCloudsData));


		// Render clouds CS
		{
			ComputePipelineDescription computeCloudsDesc = {};
			computeCloudsDesc.ComputeShader.ShaderEntryPoint = "CSClouds";
			computeCloudsDesc.ComputeShader.ShaderPath = "Clouds.hlsl";
			computeCloudsDesc.ComputeShader.Type = ShaderType::Compute;

			mCloudsPipelineCompute = mGraphicsInterface->CreateComputePipeline(computeCloudsDesc);
		}

		// Render cloud lighting
		{
			ComputePipelineDescription shadowCloudDesc = {};
			shadowCloudDesc.ComputeShader.ShaderEntryPoint = "CSCloudShadow";
			shadowCloudDesc.ComputeShader.ShaderPath = "Clouds.hlsl";
			shadowCloudDesc.ComputeShader.Type = ShaderType::Compute;

			mCloudShadowPipeline = mGraphicsInterface->CreateComputePipeline(shadowCloudDesc);
		}

		return true;
	}

	void CloudRenderer::Draw(float dt,glm::vec3 camPos, glm::mat4 iViewProj,glm::vec3 SunDirection)
	{
		Graphics::Profiler::GetInstance()->Begin("Clouds Render");
		mCloudsData.Time += dt;
		mCloudsData.ViewPosition = glm::vec4(camPos,0.0f);
		mCloudsData.InvViewProj = iViewProj;
		mCloudsData.SunDirection = glm::vec4(SunDirection, 0.0f);

		// Cloud shadow
		mGraphicsInterface->SetComputePipeline(mCloudShadowPipeline);
		mGraphicsInterface->SetResource(mCloudCoverage, 0);
		mGraphicsInterface->SetResource(mBaseNoise, 1);
		mGraphicsInterface->SetResource(mDetailNoise, 2);
		mGraphicsInterface->SetConstantBuffer(mCloudsDataHandle, 0, sizeof(mCloudsData), &mCloudsData);
		mGraphicsInterface->SetRWResource(mCloudShadowTexture, 1);
		int tx = ceil((float)128/ (float)16);
		int ty = ceil((float)128 / (float)16);
		int tz = ceil((float)32 / (float)4);
		mGraphicsInterface->Dispatch(ceil(tx),ty,tz);

		//mGraphicsInterface->SetGraphicsPipeline(mCloudsPipeline);
		//mGraphicsInterface->SetConstantBuffer(mCloudsDataHandle, 0, sizeof(mCloudsData),&mCloudsData);
		//mGraphicsInterface->SetResource(mCloudCoverage, 0);
		//mGraphicsInterface->SetResource(mBaseNoise, 1);
		//mGraphicsInterface->SetResource(mDetailNoise, 2);
		//mGraphicsInterface->SetResource(mCloudShadowTexture, 3);
		//mGraphicsInterface->Draw(6, 0);

		Graphics::Profiler::GetInstance()->End("Clouds Render");

		{
			mGraphicsInterface->SetComputePipeline(mCloudsPipelineCompute);
			mGraphicsInterface->SetConstantBuffer(mCloudsDataHandle, 0, sizeof(mCloudsData), &mCloudsData);
			mGraphicsInterface->SetResource(mCloudCoverage, 0);
			mGraphicsInterface->SetResource(mBaseNoise, 1);
			mGraphicsInterface->SetResource(mDetailNoise, 2);
			mGraphicsInterface->SetResource(mCloudShadowTexture, 3);
			mGraphicsInterface->SetRWResource(mCloudsIntermediate, 0);

			float cursizeX = mCurRenderSize.x * mDownScaleFactor;
			float cursizeY = mCurRenderSize.y * mDownScaleFactor;
			int tx = ceil((float)cursizeX / (float)32);
			int ty = ceil((float)cursizeY / (float)32);
			int tz = 1;
			mGraphicsInterface->Dispatch(tx,ty,tz);
		}

		mGraphicsInterface->SetGraphicsPipeline(mCloudsCompositePipeline);
		mGraphicsInterface->SetResource(mCloudsIntermediate, 0);
		mGraphicsInterface->Draw(6, 0);
	}

	void CloudRenderer::ShowDebug()
	{
		ImGui::Begin("Clouds");
		ImGui::DragFloat("Absorption", &mCloudsData.Absorption, 0.01f, 0.0f, 100.0f);
		ImGui::DragFloat("Cloud Base", &mCloudsData.CloudBase, 1.0f, 0.0f, 1000.0f);
		ImGui::DragFloat("Cloud extents", &mCloudsData.CloudExtents, 1.0f, 0.0f, 2000.0f);
		ImGui::InputFloat("Scale", &mCloudsData.CloudsScale);				
		if (ImGui::Button("Create Textures"))
		{
			DestroyTextures();
			CreateTextures();
		}
		if (ImGui::Button("Reload Shaders"))
		{
			mGraphicsInterface->ReloadGraphicsPipeline(mCloudsPipeline);
		}

		//ImGui::Text("Base value noise");
		//ImGui::Image((ImTextureID)mTestWorley.Handle, ImVec2(512, 512));
		//ImGui::Separator();
		ImGui::End();
	}

	void CloudRenderer::DestroyTextures()
	{
		mGraphicsInterface->ReleaseTexture(mCloudCoverage);
		mGraphicsInterface->ReleaseTexture(mBaseNoise);
		mGraphicsInterface->ReleaseTexture(mDetailNoise);
	}

	void CloudRenderer::CreateTextures()
	{
		mCloudShadowTexture = mGraphicsInterface->CreateTexture3D(128, 128, 1, 32, Graphics::Format::R_32_Float, Graphics::TextureFlags::UnorderedAccess);

		mCurRenderSize = mGraphicsInterface->GetCurrentRenderingSize();
		mCloudsIntermediate = mGraphicsInterface->CreateTexture2D(mCurRenderSize.x * mDownScaleFactor, mCurRenderSize.y * mDownScaleFactor,1,1,Graphics::Format::RGBA_16_Float,Graphics::TextureFlags::UnorderedAccess);

		struct TexelRGBA		{ uint8_t r, g, b, a; };
		struct TexelR			{ uint8_t r; };
		struct TexelRF			{ float r; };
		struct TexelR11G11B10	{uint32_t r;}; // packed!

		// Cloud coverage (2D)
		{
			int numNoiseVals = 8;
			GradientNoise2D noise2D;
			noise2D.Initialize(numNoiseVals, numNoiseVals);

			const int tw = 64;
			const int th = 64;
			TexelRGBA* texData = new TexelRGBA[tw * th];
			for (int v = 0; v < th; v++)
			{
				for (int u = 0; u < tw; u++)
				{
					float cu = float(u) / float(tw);
					float cv = float(v) / float(th);
					unsigned char n0 = unsigned char(noise2D.Sample(cu * numNoiseVals, cv * numNoiseVals) * 255.0f);
					unsigned char n = (n0);
					texData[u + v * tw] = TexelRGBA{ n,n,n, 255 };
				}
			}
			mCloudCoverage = mGraphicsInterface->CreateTexture2D(tw, th, 1, 1, Graphics::Format::RGBA_8_Unorm, TextureFlags::TextureFlagNone, texData);
			delete[] texData;
		}

		// 3D base noise
		{
			int numNoiseVals = 16;
			GradientNoise3D noise3D;
			noise3D.Initialize(numNoiseVals, numNoiseVals, numNoiseVals);

			int numNoiseValsWor = 16;
			WorleyNoise3D noise3DWorley;
			noise3DWorley.Initialize(numNoiseValsWor, numNoiseValsWor, numNoiseValsWor);

			const int tw = 64;
			const int th = 64;
			const int td = 64;
			TexelR* texData = new TexelR[tw * th * td];
			// Total size of each slice
			uint32_t slizeSize = tw * th;

			for (int w = 0; w < td; w++)			// Iterate over slices
			{
				for (int v = 0; v < th; v++)		// Top to bottom
				{
					for (int u = 0; u < tw; u++)	// Left to right
					{
						float cu = float(u) / float(tw);
						float cv = float(v) / float(th);
						float cw = float(w) / float(td);

						unsigned char r = unsigned char(cu * 255.0f);
						unsigned char g = unsigned char(cv * 255.0f);

						float n = noise3D.Fbm(cu * numNoiseVals, cv * numNoiseVals, cw * numNoiseVals, 5, 2.0f, 0.5f);
						//float wn = noise3DWorley.Fbm(cu, cv, cw, 5, 2.2f, 0.55f) * 0.25f;
						float mixedNoise = (n);

						size_t slizeOff = (slizeSize * w);
						texData[slizeOff + (u + v * tw)] = TexelR{ unsigned char(mixedNoise * 255.0f) };
					}
				}
			}

			mBaseNoise = mGraphicsInterface->CreateTexture3D(tw, th, 1, td, Graphics::Format::R_8_Unorm, TextureFlags::TextureFlagNone, texData);
			delete[] texData;
		}

		// 3D detail noise
		{
#if 0
			int numNoiseVals0 = 8;
			ValueNoise3D noise3D0;
			noise3D0.Initialize(numNoiseVals0, numNoiseVals0, numNoiseVals0);
			int numNoiseVals1 = 16;
			ValueNoise3D noise3D1;
			noise3D1.Initialize(numNoiseVals1, numNoiseVals1, numNoiseVals1);
			int numNoiseVals2 = 32;
			ValueNoise3D noise3D2;
			noise3D2.Initialize(numNoiseVals2, numNoiseVals2, numNoiseVals2);
#else
			int numNoiseVals0 = 8;
			WorleyNoise3D noise3D0;
			noise3D0.Initialize(numNoiseVals0, numNoiseVals0, numNoiseVals0);
			int numNoiseVals1 = 16;
			WorleyNoise3D noise3D1;
			noise3D1.Initialize(numNoiseVals1, numNoiseVals1, numNoiseVals1);
			int numNoiseVals2 = 32;
			WorleyNoise3D noise3D2;
			noise3D2.Initialize(numNoiseVals2, numNoiseVals2, numNoiseVals2);
#endif

			const int tw = 64;
			const int th = 64;
			const int td = 64;
			TexelR11G11B10* texData = new TexelR11G11B10[tw * th * td];
			// Total size of each slice
			uint32_t slizeSize = tw * th;

			for (int w = 0; w < td; w++)			// Iterate over slices
			{
				for (int v = 0; v < th; v++)		// Top to bottom
				{
					for (int u = 0; u < tw; u++)	// Left to right
					{
						float cu = float(u) / float(tw);
						float cv = float(v) / float(th);
						float cw = float(w) / float(td);

#if 0
						float n0 = noise3D0.Fbm(cu * float(numNoiseVals0), cv * float(numNoiseVals0), cw * float(numNoiseVals0), 4, 2.2f, 0.55f);
						float n1 = noise3D1.Fbm(cu * float(numNoiseVals1), cv * float(numNoiseVals1), cw * float(numNoiseVals1), 5, 2.2f, 0.55f);
						float n2 = noise3D2.Fbm(cu * float(numNoiseVals2), cv * float(numNoiseVals2), cw * float(numNoiseVals2), 5, 2.2f, 0.55f);
#else
						float n0 = noise3D0.Fbm(cu, cv, cw, 4, 2.2f, 0.55f);
						float n1 = noise3D1.Fbm(cu, cv, cw, 5, 2.2f, 0.55f);
						float n2 = noise3D2.Fbm(cu, cv, cw, 5, 2.2f, 0.55f);
#endif

						size_t slizeOff = (slizeSize * w);

						uint32_t packedValue = glm::packF2x11_1x10(glm::vec3(n0, n1, n2));
						texData[slizeOff + (u + v * tw)] = TexelR11G11B10 { packedValue };
					}
				}
			}

			mDetailNoise = mGraphicsInterface->CreateTexture3D(tw, th, 1, td, Graphics::Format::R_11_G_11_B_10_Float, TextureFlags::TextureFlagNone, texData);
			delete[] texData;
		}
	}
}
