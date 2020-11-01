
#include "DX12GraphicsInterface.h"
#include "Graphics/Platform/Windows/WWindow.h"
#include "Core/Logging.h"
#include "Graphics/Profiler.h"
#include "Core/FileSystem.h"

#include "Graphics/UI/IMGUI/imgui.h"

#include <iostream>
#include <vector>

namespace Graphics { namespace DX12 {

	DX12GraphicsInterface::DX12GraphicsInterface():
		mDevice(nullptr),
		mGraphicsRootSignature(nullptr),
		mFrameHeap(nullptr),
		mFrame(0),
		mCurBackBuffer(0),
		mNumDrawCalls(0),
		mReleaseManager(this),
		mTimeStampsHeap(nullptr),
		mTimeStampsMemory(nullptr)
	{
	}
	
	DX12GraphicsInterface::~DX12GraphicsInterface()
	{
	}

	bool DX12GraphicsInterface::Initialize(Platform::BaseWindow* targetWindow)
	{
		// Provide graphics to the profiler
		Profiler::GetInstance()->Init(this);

		HWND whandle = (HWND)targetWindow;
		memset(&mDefaultSurface, 0, sizeof(DisplaySurface));
		mDefaultSurface.Window = (Platform::Windows::WWindow*)targetWindow;
		mOutputFormat = Format::RGBA_8_Unorm;
		 
		UINT factoryFlags = 0;
		ID3D12Debug* debugController = nullptr;
		static bool enableDebug = false;
		if (enableDebug && SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			static bool enableGPUValidation = false;
			if (enableGPUValidation)
			{
				ID3D12Debug1* debug1 = nullptr;
				D3D12GetDebugInterface(IID_PPV_ARGS(&debug1));
				debug1->SetEnableGPUBasedValidation(true);
				debug1->SetEnableSynchronizedCommandQueueValidation(true);
			}
			debugController->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			debugController->Release();
		}

		// Query adapters:
		IDXGIFactory4* factory = nullptr;
		if (FAILED(CreateDXGIFactory2(factoryFlags,IID_PPV_ARGS(&factory))))
		{
			std::cerr << "Could not create a factory! \n";
			return false;
		}
		{
			int aIdx = 0;
			bool found = false;
			while (factory->EnumAdapters1(aIdx++, &mDefaultSurface.GPU) != DXGI_ERROR_NOT_FOUND)
			{
				DXGI_ADAPTER_DESC1 adesc = {};
				mDefaultSurface.GPU->GetDesc1(&adesc);
				if (adesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					continue;
				}
				if (SUCCEEDED(D3D12CreateDevice(mDefaultSurface.GPU, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				std::cerr << "Could not find an adapter! \n";
				return false;
			}
		}
		factory->Release();

		// Create device:
		if (FAILED(D3D12CreateDevice(mDefaultSurface.GPU, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice))))
		{
			std::cerr << "Could not create the device! \n";
			return false;
		}
		// Query features
		mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &mDeviceFeatures, sizeof(mDeviceFeatures));
		if (mDeviceFeatures.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1)
		{
			INFO("Device binding tier 1");
		}
		else if (mDeviceFeatures.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_2)
		{
			INFO("Device binding tier 2");
		}
		else
		{
			INFO("Device binding tier 3");
		}

		// Mute some annoyances:
		ID3D12InfoQueue* infoQueue = nullptr;
		mDevice->QueryInterface(IID_PPV_ARGS(&infoQueue));
		if (infoQueue)
		{
			D3D12_INFO_QUEUE_FILTER filter = {};
			D3D12_MESSAGE_ID idList[] =
			{
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE
			};
			filter.DenyList.NumIDs = sizeof(idList) / sizeof(D3D12_MESSAGE_ID);
			filter.DenyList.pIDList = idList;
			infoQueue->AddStorageFilterEntries(&filter);
			infoQueue->Release();
		}

		// Initialize the display surface:
		InitSurface(&mDefaultSurface);

		InitRootSignature();
		
		// Time stamp heap
		D3D12_QUERY_HEAP_DESC tsHeapDesc = {};
		tsHeapDesc.Count = 8000;
		tsHeapDesc.NodeMask = 0;
		tsHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		mDevice->CreateQueryHeap(&tsHeapDesc, IID_PPV_ARGS(&mTimeStampsHeap));
		// Time stamps memory
		// We just map it once and roll with it
		// Also, note that we create up to NUM_BACK_BUFFERS!
		CD3DX12_RESOURCE_DESC memTs = CD3DX12_RESOURCE_DESC::Buffer(8000 * 8 * NUM_BACK_BUFFERS, D3D12_RESOURCE_FLAG_NONE);
		D3D12_HEAP_PROPERTIES memHeap = CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
		mDevice->CreateCommittedResource(&memHeap, D3D12_HEAP_FLAG_NONE, &memTs, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&mTimeStampsMemory));
		mTimeStampsMemory->Map(0, nullptr, reinterpret_cast<void**>(&mTimeStampMemPtr));

		mViewsHeap.Initialize(mDevice, 4096, false, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Null resources
		mNullsHeap.Initialize(mDevice, 4, false, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC nullUav = {};
			nullUav.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			nullUav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D; 
			mNullUav = mNullsHeap.GetCPU();
			mDevice->CreateUnorderedAccessView(nullptr, nullptr, &nullUav, mNullUav);
			mNullsHeap.OffsetHandles(1);
		}
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC nullSrv = {};
			nullSrv.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			nullSrv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			nullSrv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			mNullSrv = mNullsHeap.GetCPU();
			mDevice->CreateShaderResourceView(nullptr, &nullSrv, mNullSrv);
			mNullsHeap.OffsetHandles(1);
		}
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC nullCbv = {};
			mNullCbv = mNullsHeap.GetCPU();
			mDevice->CreateConstantBufferView(&nullCbv, mNullCbv);
			mNullsHeap.OffsetHandles(1);
		}
		return true;
	}

	void DX12GraphicsInterface::InitSurface(DisplaySurface* surface)
	{
		// Command queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.NodeMask	= 0;
		queueDesc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Type		= D3D12_COMMAND_LIST_TYPE_DIRECT;
		mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&surface->Queue));

		// Swap chain
		auto width	= surface->Window->GetWidth();
		auto height = surface->Window->GetHeight();
		auto handle = (HWND)surface->Window->GetHandle();

		DXGI_MODE_DESC bufferDesc = {};
		bufferDesc.Width					= width;
		bufferDesc.Height					= height;
		bufferDesc.Format					= ToDXGIFormat(mOutputFormat);
		bufferDesc.ScanlineOrdering			= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		bufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;
		bufferDesc.RefreshRate.Numerator	= 1;
		bufferDesc.RefreshRate.Denominator	= 0;

		DXGI_SWAP_CHAIN_DESC chainDesc	= {};
		chainDesc.BufferDesc			= bufferDesc;
		chainDesc.BufferCount			= NUM_BACK_BUFFERS;
		chainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT; 
		chainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD; 
		chainDesc.OutputWindow			= handle; 
		chainDesc.SampleDesc.Count		= 1;
		chainDesc.SampleDesc.Quality	= 0;
		chainDesc.Windowed				= !surface->Window->IsFullScreen();

		IDXGISwapChain* tmpChain	= nullptr;
		IDXGIFactory4* factory		= nullptr;
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
		{
			return;
		}
		if (FAILED(factory->CreateSwapChain(surface->Queue, &chainDesc, &tmpChain)))
		{
			return;
		}
		factory->Release();
		surface->SwapChain = static_cast<IDXGISwapChain3*>(tmpChain);
		
		// Create rt views
		// Create cmd allocators
		// Create fences
		D3D12_DESCRIPTOR_HEAP_DESC rtHeapDesc = {};
		rtHeapDesc.NumDescriptors	= NUM_BACK_BUFFERS;
		rtHeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // This disables shader access
		rtHeapDesc.NodeMask			= 0;
		rtHeapDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		mDevice->CreateDescriptorHeap(&rtHeapDesc, IID_PPV_ARGS(&surface->Heap));
		auto curHandle  = surface->Heap->GetCPUDescriptorHandleForHeapStart();
		auto handleSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for (int i = 0; i < NUM_BACK_BUFFERS; i++)
		{
			surface->SwapChain->GetBuffer(i, IID_PPV_ARGS(&surface->BackBuffers[i]));

			// Name back buffer:
			std::string name = "Back Buffer ";
			name += std::to_string(i);
			surface->BackBuffers[i]->SetName(std::wstring(name.begin(), name.end()).c_str());

			mDevice->CreateRenderTargetView(surface->BackBuffers[i], nullptr, curHandle);
			surface->RenderTargets[i] = curHandle;
			curHandle.ptr += handleSize;

			mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&surface->Allocators[i]));

			mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&surface->GPUFences[i]));
			surface->GPUFencesValues[i] = 0;
		}
		// Fence event
		surface->GPUFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		// Create a cmd list
		mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, surface->Allocators[0], nullptr, IID_PPV_ARGS(&surface->CmdContext));
		surface->Recording = true;
	}

	void DX12GraphicsInterface::InitRootSignature()
	{
		CD3DX12_ROOT_SIGNATURE_DESC rsDesc = {};
		std::vector<CD3DX12_ROOT_PARAMETER> params;
		std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplers;

		// param 0 (CBV) & (TEX) & (UAV)
		CD3DX12_DESCRIPTOR_RANGE descriptorRanges[3];
		{
			CD3DX12_ROOT_PARAMETER rootParam;
			descriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NUM_CBVS, 0);
			descriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, NUM_SRVS, 0, 0, NUM_CBVS);
			descriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, NUM_UAVS, 0, 0, NUM_CBVS + NUM_SRVS);
			rootParam.InitAsDescriptorTable(sizeof(descriptorRanges) / sizeof(CD3DX12_DESCRIPTOR_RANGE), descriptorRanges);
			params.push_back(rootParam);
		}
		// LineaWrapSampler (s0)
		{
			CD3DX12_STATIC_SAMPLER_DESC s0;
			s0.Init
			(
				0,
				D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR
			);
			samplers.push_back(s0);
		}
		// LinearClampSampler (s1)
		{
			CD3DX12_STATIC_SAMPLER_DESC s1;
			s1.Init
			(
				1,
				D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP
			);
			samplers.push_back(s1);
		}
		rsDesc.Init((UINT)params.size(), params.data(), (UINT)samplers.size(),samplers.data());
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ID3D10Blob* rsBlob = nullptr;
		ID3D10Blob* rsErrorBlob = nullptr;
		if (FAILED(D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &rsErrorBlob)))
		{
			OutputDebugStringA((char*)rsErrorBlob->GetBufferPointer());
		}
		mDevice->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&mGraphicsRootSignature));

		// Lets init the shader visible heap
		mFrameHeap = new DX12Heap*[NUM_BACK_BUFFERS];
		for (int i = 0; i < NUM_BACK_BUFFERS; i++)
		{
			mFrameHeap[i] = new DX12Heap;
			mFrameHeap[i]->Initialize(mDevice, 1000000, true, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			// Name this heap:
			std::string name = "Frame Heap ";
			name += std::to_string(i);
			std::wstring wname = std::wstring(name.begin(), name.end());
			mFrameHeap[i]->GetHeap()->SetName(wname.c_str());
		}

		// And some storage heaps
		mRenderTargetHeap = new DX12Heap;
		mRenderTargetHeap->Initialize(mDevice, 512, false, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		mDepthStencilHeap = new DX12Heap;
		mDepthStencilHeap->Initialize(mDevice, 512, false, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	bool DX12GraphicsInterface::LoadShader(const ShaderDescription& desc, D3D12_SHADER_BYTECODE& outShader)
	{
		// Load file
		// FILE* shaderFile = nullptr;
		// fopen_s(&shaderFile, path.c_str(), "r");
		// fseek(shaderFile, 0, SEEK_END);
		// auto size = ftell(shaderFile);
		// fseek(shaderFile, 0, SEEK_SET);
		// char* src = (char*)malloc(size);
		// memset(src, 0, size);
		// fread_s(src, size, sizeof(char), size / sizeof(char), shaderFile);
		// fclose(shaderFile);

		// Shader
		ID3D10Blob* error;
		ID3D10Blob* sblob;

		std::string fixedupPath = desc.ShaderPath;
		if (!Core::FileSystem::GetInstance()->FixupPath(fixedupPath))
		{
			ERR("Could not Fixup path: %s", fixedupPath.c_str());
			assert(false); // For now, just hard fail.
			return false;
		}

		std::wstring wpath(fixedupPath.begin(), fixedupPath.end());
		std::string target;
		switch (desc.Type)
		{
		case ShaderType::Vertex:	target = "vs_5_0"; break;
		case ShaderType::Pixel:		target = "ps_5_0"; break;
		case ShaderType::Compute:	target = "cs_5_0"; break;
		default:					target = "none_5_0"; break;
		}
		//UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_VALIDATION;// | D3DCOMPILE_OPTIMIZATION_LEVEL3;
		UINT flags = D3DCOMPILE_DEBUG;// | D3DCOMPILE_OPTIMIZATION_LEVEL3;
		if (FAILED(D3DCompileFromFile(wpath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, desc.ShaderEntryPoint.c_str(), target.c_str(), flags, 0, &sblob, &error)))
		{
			OutputDebugStringA((char*)error->GetBufferPointer());
 			assert(false);
			return false;
		}
		outShader.BytecodeLength = sblob->GetBufferSize();
		outShader.pShaderBytecode = sblob->GetBufferPointer();
		return true;
	}

	void DX12GraphicsInterface::FlushHeap(bool graphics /*=true*/)
	{
		if (mBindingState.Dirty)
		{
			UINT idx = mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();
			mBindingState.Dirty = false;

			// Start by copying the new descriptors:
			auto destHandle = mFrameHeap[idx]->GetCPU();
			for (int cbslot = 0; cbslot < NUM_CBVS; ++cbslot)
			{
				if (!mBindingState.CBSlots[cbslot].Null)
				{
					mDevice->CopyDescriptorsSimple(1, destHandle, mBindingState.CBSlots[cbslot].CPUView, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}
				destHandle.Offset(1, mFrameHeap[idx]->GetIncrementSize());
			}

			for (int srslot = 0; srslot < NUM_SRVS; ++srslot)
			{
				if (!mBindingState.SRSlots[srslot].Null)
				{
					mDevice->CopyDescriptorsSimple(1, destHandle, mBindingState.SRSlots[srslot].CPUView, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}
				destHandle.Offset(1, mFrameHeap[idx]->GetIncrementSize());
			}

			for (int uaslot = 0; uaslot < NUM_UAVS; ++uaslot)
			{
				if (!mBindingState.UASlots[uaslot].Null)
				{
					mDevice->CopyDescriptorsSimple(1, destHandle, mBindingState.UASlots[uaslot].CPUView, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				}
				destHandle.Offset(1, mFrameHeap[idx]->GetIncrementSize());
			}


			// Now set the root parameter:
			if (graphics)
			{
				mDefaultSurface.CmdContext->SetGraphicsRootDescriptorTable(0, mFrameHeap[idx]->GetGPU());
			}
			else
			{
				mDefaultSurface.CmdContext->SetComputeRootDescriptorTable(0, mFrameHeap[idx]->GetGPU());
			}

			// Finally offset the handles:
			mFrameHeap[idx]->OffsetHandles(NUM_SRVS + NUM_UAVS + NUM_CBVS);
		}
	}

	void DX12GraphicsInterface::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, bool forceFlush /*=false*/, uint32_t subResource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, after, subResource);
		mPendingBarriers.push_back(barrier);
		if (forceFlush)
		{
			FlushBarriers();	
		}
	}

	void DX12GraphicsInterface::FlushBarriers()
	{
		UINT numBarriers = (UINT)mPendingBarriers.size();
		if (numBarriers > 0)
		{
			mDefaultSurface.CmdContext->ResourceBarrier(numBarriers, mPendingBarriers.data());
			mPendingBarriers.clear();
		}
	}

	void DX12GraphicsInterface::CreatePSO(const GraphicsPipelineDescription& desc, GraphicsPipelineEntry& entry)
	{
		// Build the input layout:
		D3D12_INPUT_LAYOUT_DESC inputL = {};
		std::vector<D3D12_INPUT_ELEMENT_DESC> eles;
		eles.resize(desc.VertexDescription.NumElements);
		for (int i = 0; i < desc.VertexDescription.NumElements; i++)
		{
			eles[i].SemanticName = desc.VertexDescription.Elements[i].Semantic;
			eles[i].SemanticIndex = desc.VertexDescription.Elements[i].Idx;
			eles[i].Format = ToDXGIFormat(desc.VertexDescription.Elements[i].EleFormat);
			eles[i].AlignedByteOffset = (UINT)desc.VertexDescription.Elements[i].Offset;
			eles[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			eles[i].InstanceDataStepRate = 0;
			eles[i].InputSlot = 0;
		}
		inputL.NumElements = desc.VertexDescription.NumElements;
		inputL.pInputElementDescs = eles.data();

		// Describe the pso
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		LoadShader(desc.PixelShader, psoDesc.PS);
		LoadShader(desc.VertexShader, psoDesc.VS);

		// Blend info
		{
			psoDesc.BlendState = CD3DX12_BLEND_DESC::CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			for (int i = 0; i < 8; i++)
			{
				auto& cur = psoDesc.BlendState.RenderTarget[i];
				auto& curDesc = desc.BlendTargets[i];

				cur.RenderTargetWriteMask = curDesc.WriteMask;
				cur.BlendEnable = curDesc.Enabled;
				if (curDesc.Enabled)
				{
					cur.SrcBlend = ToDX12Blend(curDesc.SrcBlendColor);
					cur.DestBlend = ToDX12Blend(curDesc.DstBlendColor);
					cur.BlendOp = ToDX12BlendOp(curDesc.BlendOpColor);

					cur.SrcBlendAlpha = ToDX12Blend(curDesc.SrcBlendAlpha);
					cur.DestBlendAlpha = ToDX12Blend(curDesc.DstBlendAlpha);
					cur.BlendOpAlpha = ToDX12BlendOp(curDesc.BlendOpAlpha);
				}
			}
		}
		// Depth info
		{
			psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC::CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			psoDesc.DepthStencilState.DepthEnable = desc.DepthEnabled;
			psoDesc.DepthStencilState.DepthFunc = ToDX12DepthFunc(desc.DepthFunction);
			psoDesc.DepthStencilState.DepthWriteMask = desc.DepthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
			psoDesc.DSVFormat = ToDXGIFormat(desc.DepthFormat);
		}
		// Raster info
		{
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC::CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.RasterizerState.CullMode = ToDX12FaceCullMode(desc.CullMode);
			psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		}
		psoDesc.InputLayout = inputL;
		psoDesc.pRootSignature = mGraphicsRootSignature;
		int numTargets = 0;
		for (int i = 0; i < 8; i++)
		{
			if (desc.ColorFormats[i] != Format::Unknown)
			{
				psoDesc.RTVFormats[i] = ToDXGIFormat(desc.ColorFormats[i]);
				psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				numTargets++;
			}
		}
		psoDesc.NumRenderTargets = numTargets;
		psoDesc.PrimitiveTopologyType = ToDXGIPrimitive(desc.PrimitiveType);
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.SampleMask = 0xffffffff;

		if (desc.PrimitiveType == Primitive::Line)
		{
			psoDesc.RasterizerState.AntialiasedLineEnable = true;
		}

		mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&entry.Pso));
	}

	void DX12GraphicsInterface::CreatePSO(const ComputePipelineDescription& desc, ComputePipelineEntry& entry)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = mGraphicsRootSignature;
		LoadShader(desc.ComputeShader, psoDesc.CS);
		mDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&entry.Pso));
	}

	DXGI_FORMAT DX12GraphicsInterface::ToDXGIFormat(const Graphics::Format& format)
	{
		switch (format)
		{
			case Format::RG_32_Float:			return DXGI_FORMAT_R32G32_FLOAT;
			case Format::RGB_32_Float:			return DXGI_FORMAT_R32G32B32_FLOAT; 
			case Format::RGBA_32_Float:			return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case Format::Depth24_Stencil8:		return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case Format::RGBA_8_Unorm:			return DXGI_FORMAT_R8G8B8A8_UNORM;
			case Format::RGBA_16_Float:			return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case Format::RGBA_8_Snorm:			return DXGI_FORMAT_R8G8B8A8_SNORM;
			case Format::R_16_Uint:				return DXGI_FORMAT_R16_UINT;
			case Format::R_32_Uint:				return DXGI_FORMAT_R32_UINT;
			case Format::R_8_Unorm:				return DXGI_FORMAT_R8_UNORM;
			case Format::R_32_Float:			return DXGI_FORMAT_R32_FLOAT;
			case Format::R_11_G_11_B_10_Float:	return DXGI_FORMAT_R11G11B10_FLOAT;
			case Format::Unknown:
			default:							return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT DX12GraphicsInterface::ToDXGIFormatTypeless(const Graphics::Format & format)
	{
		switch (format)
		{
		case Format::RG_32_Float:			return DXGI_FORMAT_R32G32_TYPELESS;
		case Format::RGB_32_Float:			return DXGI_FORMAT_R32G32B32_TYPELESS;
		case Format::RGBA_32_Float:			return DXGI_FORMAT_R32G32B32A32_TYPELESS;
		case Format::Depth24_Stencil8:		return DXGI_FORMAT_R24G8_TYPELESS;
		case Format::RGBA_8_Unorm:			return DXGI_FORMAT_R8G8B8A8_TYPELESS;
		case Format::RGBA_16_Float:			return DXGI_FORMAT_R16G16B16A16_TYPELESS;
		case Format::RGBA_8_Snorm:			return DXGI_FORMAT_R8G8B8A8_TYPELESS;
		case Format::R_16_Uint:				return DXGI_FORMAT_R16_TYPELESS;
		case Format::R_32_Uint:				return DXGI_FORMAT_R32_TYPELESS;
		case Format::R_8_Unorm:				return DXGI_FORMAT_R8_TYPELESS;
		case Format::R_32_Float:			return DXGI_FORMAT_R32_TYPELESS;
		case Format::R_11_G_11_B_10_Float:	return DXGI_FORMAT_R11G11B10_FLOAT; // ????? R32 maybe
		case Format::Unknown:
		default:							return DXGI_FORMAT_UNKNOWN;
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY DX12GraphicsInterface::ToDXGITopology(const Topology& topology)
	{
		switch (topology)
		{
			case Topology::TriangleList: return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case Topology::LineList:	 return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case Topology::LineStrip:	 return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case Topology::InvalidTopology:
			default: return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE DX12GraphicsInterface::ToDXGIPrimitive(const Graphics::Primitive::T& primitive)
	{
		switch (primitive)
		{
			case Primitive::Point:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			case Primitive::Line:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			case Primitive::Triangle:	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			case Primitive::Patch:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			case Primitive::Undefined:
			default:
							return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
	}

	D3D12_COMPARISON_FUNC DX12GraphicsInterface::ToDX12DepthFunc(const DepthFunc::T& func)
	{
		switch (func)
		{
			case DepthFunc::Always:		return D3D12_COMPARISON_FUNC_ALWAYS;
			case DepthFunc::Equal:		return D3D12_COMPARISON_FUNC_EQUAL;
			case DepthFunc::GreatEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case DepthFunc::LessEqual:	return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case DepthFunc::Never:		return D3D12_COMPARISON_FUNC_NEVER;
			default:					return D3D12_COMPARISON_FUNC_ALWAYS;
		}
	}

	D3D12_CULL_MODE DX12GraphicsInterface::ToDX12FaceCullMode(const FaceCullMode::T& mode)
	{
		switch (mode)
		{
			case FaceCullMode::Front: return D3D12_CULL_MODE_FRONT;
			case FaceCullMode::Back: return D3D12_CULL_MODE_BACK;
			default:
			case FaceCullMode::None: return D3D12_CULL_MODE_NONE;
		}
	}

	D3D12_BLEND DX12GraphicsInterface::ToDX12Blend(const BlendFunction::T& func)
	{
		switch (func)
		{
			case BlendFunction::BlendZero:			return D3D12_BLEND_ZERO;
			case BlendFunction::BlendOne:			return D3D12_BLEND_ONE;
			case BlendFunction::BlendSrcColor:		return D3D12_BLEND_SRC_COLOR;
			case BlendFunction::BlendInvSrcColor:	return D3D12_BLEND_INV_SRC_COLOR;
			case BlendFunction::BlendSrcAlpha:		return D3D12_BLEND_SRC_ALPHA;
			case BlendFunction::BlendInvSrcAlpha:	return D3D12_BLEND_INV_SRC_ALPHA;
			case BlendFunction::BlendDstAlpha:		return D3D12_BLEND_DEST_ALPHA;
			case BlendFunction::BlendInvDstAlpha:	return D3D12_BLEND_INV_DEST_ALPHA;
			case BlendFunction::BlendDstColor:		return D3D12_BLEND_DEST_COLOR;
			case BlendFunction::BlendInvDstColor:	return D3D12_BLEND_INV_DEST_COLOR;
			case BlendFunction::BlendFactor:		return D3D12_BLEND_BLEND_FACTOR;
			default:								return D3D12_BLEND_ZERO;
		}
	}

	D3D12_BLEND_OP DX12GraphicsInterface::ToDX12BlendOp(const BlendOperation::T& op)
	{
		switch (op)
		{
			case BlendOperation::BlendOpAdd:		return D3D12_BLEND_OP_ADD;
			case BlendOperation::BlendOpSubstract:	return D3D12_BLEND_OP_SUBTRACT;
			case BlendOperation::BlendOpMin:		return D3D12_BLEND_OP_MIN;
			case BlendOperation::BlendOpMax:		return D3D12_BLEND_OP_MAX;
			default:								return D3D12_BLEND_OP_ADD;
		}
	}

	void DX12GraphicsInterface::StartFrame()
	{
		if (mDefaultSurface.Recording)
		{
			std::cout << "We are recording commands!!!\n";
		}

		mCurBackBuffer = mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();

		// Lets start by making sure it is safe to write to the current buffer
		UINT idx = mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();
		if (mDefaultSurface.GPUFences[idx]->GetCompletedValue() < mDefaultSurface.GPUFencesValues[idx])
		{
			mDefaultSurface.GPUFences[idx]->SetEventOnCompletion(mDefaultSurface.GPUFencesValues[idx], mDefaultSurface.GPUFenceEvent);
			WaitForSingleObject(mDefaultSurface.GPUFenceEvent, INFINITE);
		}

		// Now its a good time to update the release manager:
		mReleaseManager.Update();

		// Reset allocator and get the default context ready
		mDefaultSurface.Allocators[idx]->Reset();
		auto context = mDefaultSurface.CmdContext;
		context->Reset(mDefaultSurface.Allocators[idx], nullptr);		
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition
		(
			mDefaultSurface.BackBuffers[idx], 
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		context->ResourceBarrier(1, &barrier);
		context->OMSetRenderTargets(1, &mDefaultSurface.RenderTargets[idx], false, nullptr);
		float clear[4]{ 0.2f,0.2f,0.25f,1.0f };
		context->ClearRenderTargetView(mDefaultSurface.RenderTargets[idx], clear, 0, nullptr);
		D3D12_VIEWPORT vp	= {};
		vp.TopLeftX			= vp.TopLeftY = 0.0f;
		vp.Width			= (FLOAT)mDefaultSurface.Window->GetWidth();
		vp.Height			= (FLOAT)mDefaultSurface.Window->GetHeight();
		vp.MinDepth			= 0.0f;
		vp.MaxDepth			= 1.0f;
		context->RSSetViewports(1, &vp);

		// Bind frame heap
		mFrameHeap[idx]->Reset();
		ID3D12DescriptorHeap* heaps[] = { mFrameHeap[idx]->GetHeap() };
		context->SetDescriptorHeaps(1, heaps);

		// New heap, so make sure we reset the binding state:
		mBindingState.Reset();

		mNumDrawCalls = 0;
	}

	void DX12GraphicsInterface::RenderUI()
	{
		if (ImGui::BeginMainMenuBar())
		{
			std::string filter;
			filter.resize(512);
			if (ImGui::BeginMenu("Shaders"))
			{
				ImGui::InputText("Shader Filter", &filter[0], 512);
				if (ImGui::MenuItem("Recompile shader(s)!"))
				{
					mGraphicsPipelinesPool.ForEachEntry([this,filter](GraphicsPipelineEntry& entry) {
						bool recompile = true;
						if (!filter.empty())
						{
							recompile = false;
							if (filter.find(entry.Desc.VertexShader.ShaderEntryPoint.c_str()) || filter.find(entry.Desc.PixelShader.ShaderEntryPoint.c_str()))
							{
								recompile = true;
							}
						}
						if (recompile)
						{
							INFO("Recompiling PSO: VS(%s) PS(%s)", entry.Desc.VertexShader.ShaderEntryPoint.c_str(), entry.Desc.PixelShader.ShaderEntryPoint.c_str());
							mReleaseManager.ReleaseItem(entry.Pso);
							entry.Pso = nullptr;
							CreatePSO(entry.Desc, entry);
						}
					});
					mComputePipelinesPool.ForEachEntry([this, filter](ComputePipelineEntry& entry) {
						bool recompile = true;
						if (recompile)
						{
							INFO("Recompiling PSO: CS(%s)", entry.Desc.ComputeShader.ShaderEntryPoint.c_str());
							mReleaseManager.ReleaseItem(entry.Pso);
							entry.Pso = nullptr;
							CreatePSO(entry.Desc, entry);
						}
					});
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void DX12GraphicsInterface::EndFrame()
	{
		// Get ready to present
		UINT idx		= mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();
		auto context	= mDefaultSurface.CmdContext;
		auto barrier	= CD3DX12_RESOURCE_BARRIER::Transition
		(
			mDefaultSurface.BackBuffers[idx],
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);
		context->ResourceBarrier(1, &barrier);
		context->Close();
		mDefaultSurface.Recording = false;

		ID3D12CommandList* cmdLists[] = {context};
		mDefaultSurface.Queue->ExecuteCommandLists(1, cmdLists);
		mDefaultSurface.SwapChain->Present(0, 0);
		mDefaultSurface.GPUFencesValues[idx]++;
		mDefaultSurface.Queue->Signal(mDefaultSurface.GPUFences[idx], mDefaultSurface.GPUFencesValues[idx]);

		mFrame++;
	}

	void DX12GraphicsInterface::FlushAndWait()
	{
		// Submit commands if needed first
		if (mDefaultSurface.Recording)
		{
			mDefaultSurface.CmdContext->Close();
			mDefaultSurface.Recording = false;
			ID3D12CommandList* cmdLists[] = { mDefaultSurface.CmdContext };
			mDefaultSurface.Queue->ExecuteCommandLists(1, cmdLists);
		}

		// Now lets wait
		ID3D12Fence* tmpFence;
		mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&tmpFence));
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		mDefaultSurface.Queue->Signal(tmpFence, 1);
		if (tmpFence->GetCompletedValue() != 1)
		{
			tmpFence->SetEventOnCompletion(1, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
		}
		tmpFence->Release();
	}

	BufferHandle DX12GraphicsInterface::CreateBuffer(BufferType type, CPUAccess::T cpuAccess, GPUAccess::T gpuAccess, uint64_t size, uint32_t stride /*= 0*/, void* data /*= nullptr*/)
	{
		// TO-DO: If CPU read none (static buffer) we could delete upload buffer once done with it.

		bool isIndex = type == BufferType::IndexBuffer;
		bool isStructuredBuffer = type == BufferType::GPUBuffer;
		bool isGPURw = gpuAccess == GPUAccess::ReadWrite;
		bool isCPUWrite = cpuAccess == CPUAccess::Write;

		BufferHandle handle;
		BufferEntry& bufferEntry = mBuffersPool.GetFreeEntry(handle.Handle);
		bufferEntry.Type = type;
		bufferEntry.CPUAccessMode = cpuAccess;
		bufferEntry.GPUAccessMode = gpuAccess;
		bufferEntry.LastFrame = mFrame;
		if (isStructuredBuffer)
		{
			bufferEntry.State = isGPURw ? UNORDERED_ACCESS : SRV_READ;
		}
		else
		{
			bufferEntry.State = isIndex ? INDEX_READ : VERTEX_CB_READ;
		}


		// Size should be multiple of 256
		if (type == BufferType::ConstantBuffer)
		{
			size = ((size + 255) & ~255);
		}

		uint64_t resourceSize = isStructuredBuffer ? size * stride : size;

		// GPU resource
		D3D12_HEAP_PROPERTIES heapDesc = {};
		auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(resourceSize);
		desc.Flags = isGPURw ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
		mDevice->CreateCommittedResource
		(
			&heapProp, 
			D3D12_HEAP_FLAG_NONE, 
			&desc, 
			bufferEntry.State,
			nullptr, 
			IID_PPV_ARGS(&bufferEntry.Buffer)
		);
		
		// Upload resource
		heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		desc = CD3DX12_RESOURCE_DESC::Buffer(resourceSize);
		if (type == BufferType::ConstantBuffer)
		{
			desc = CD3DX12_RESOURCE_DESC::Buffer(CB_INTERMIDIATE_SIZE);

			// Also create a view for the resource:
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = bufferEntry.Buffer->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = (UINT)resourceSize;
			bufferEntry.CBV = mViewsHeap.GetCPU();
			mDevice->CreateConstantBufferView(&cbvDesc, bufferEntry.CBV);
			mViewsHeap.OffsetHandles(1);
		}
		else if (type == BufferType::GPUBuffer)
		{
			// Create views:
			if (gpuAccess == GPUAccess::Read || gpuAccess == GPUAccess::ReadWrite)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				srvDesc.Format = DXGI_FORMAT_UNKNOWN;
				srvDesc.Buffer.FirstElement = 0;
				srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
				srvDesc.Buffer.NumElements = (UINT)size;
				srvDesc.Buffer.StructureByteStride = stride;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				bufferEntry.GPUBufferView = mViewsHeap.GetCPU();
				mDevice->CreateShaderResourceView(bufferEntry.Buffer, &srvDesc, bufferEntry.GPUBufferView);
				mViewsHeap.OffsetHandles(1);
			}
			if (gpuAccess == GPUAccess::ReadWrite)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;
				uavDesc.Buffer.FirstElement = 0;
				uavDesc.Buffer.NumElements = (UINT)size;
				uavDesc.Buffer.StructureByteStride = stride;
				uavDesc.Buffer.CounterOffsetInBytes = 0;
				uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
				bufferEntry.GPURWBufferView = mViewsHeap.GetCPU();
				mDevice->CreateUnorderedAccessView(bufferEntry.Buffer, nullptr, &uavDesc, bufferEntry.GPURWBufferView);
				mViewsHeap.OffsetHandles(1);
			}
		}
		if (type != BufferType::ConstantBuffer && isCPUWrite)
		{
			// CPUAccess::Write, we can now Map the buffer, but we need to do it
			// in a safer way so we do not override data
			desc.Width *= NUM_BACK_BUFFERS;
		}
		mDevice->CreateCommittedResource
		(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&bufferEntry.UploadHeap)
		);
		
		if (data && type != BufferType::ConstantBuffer)
		{
			SetBufferData(handle, (int)resourceSize, 0, data);
		}
		
		return handle;
	}

	TextureHandle DX12GraphicsInterface::CreateTexture2D(uint32_t width, uint32_t height, uint32_t mips, uint32_t layers, Format format, TextureFlags::T flags /* = TextureFlagNone*/, void* data /*= nullptr*/)
	{
		if ((width * height * layers * mips) == 0)
		{
			return InvalidTexture;
		}

		TextureHandle handle;
		TextureEntry& curTexEntry = mTexturesPool.GetFreeEntry(handle.Handle);

		bool isUav = false;

		D3D12_RESOURCE_FLAGS dxflags = D3D12_RESOURCE_FLAG_NONE;
		if ((flags & TextureFlags::UnorderedAccess) == TextureFlags::UnorderedAccess)
		{
			isUav = true;
			dxflags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}
		if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget)
		{
			dxflags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}
		if ((flags & TextureFlags::DepthStencil) == TextureFlags::DepthStencil)
		{
			dxflags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		}
		CD3DX12_RESOURCE_DESC texDesc;
		texDesc = CD3DX12_RESOURCE_DESC::Tex2D
		(
			ToDXGIFormatTypeless(format),
			width,
			height,
			layers,
			mips,
			1, 0,
			dxflags
		);
		CD3DX12_HEAP_PROPERTIES heapP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto& state = curTexEntry.State;
		state = isUav ? UNORDERED_ACCESS : SRV_READ;
		mDevice->CreateCommittedResource
		(
			&heapP, D3D12_HEAP_FLAG_NONE,
			&texDesc, state,
			nullptr,
			IID_PPV_ARGS(&curTexEntry.Resource)
		);

		// Here we cache a few values that we will use to upload the mips:
		UINT64 totalSize;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* footPrints = new D3D12_PLACED_SUBRESOURCE_FOOTPRINT[mips];
		UINT64* rowSizes = new UINT64[mips];
		mDevice->GetCopyableFootprints(&texDesc, 0, mips, 0, footPrints, nullptr, rowSizes, &totalSize);

		// Upload buffer
		// Sometimes the following CreateCommittedResource call will fail with "wrong memory preference..." 
		// setting again fixes it. Prob because it is a static member¿??¿
		heapP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		mDevice->CreateCommittedResource
		(
			&heapP, D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(totalSize), D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&curTexEntry.UploadHeap)
		);
		if (data)
		{
			auto baseDesc = curTexEntry.Resource->GetDesc();
			uint64_t curOff = 0;
			for (uint32_t i = 0; i < mips; i++)
			{
				const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& curPrint = footPrints[i];

				UINT64 curSliceSize = rowSizes[i] * curPrint.Footprint.Height;

				D3D12_SUBRESOURCE_DATA d = {};
				d.pData = (void*)(curOff + (unsigned char*)data);
				d.RowPitch = rowSizes[i];
				d.SlicePitch = curSliceSize;

				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(curTexEntry.Resource, state, COPY_DST));
				UpdateSubresources(mDefaultSurface.CmdContext, curTexEntry.Resource, curTexEntry.UploadHeap, curPrint.Offset, i, 1, &d);
				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(curTexEntry.Resource, COPY_DST, state));

				curOff += rowSizes[i] * curPrint.Footprint.Height;
			}
		}

		delete[] footPrints;
		delete[] rowSizes;

		// Render target views
		if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtDesc = {};
			rtDesc.Format = ToDXGIFormat(format);
			rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtDesc.Texture2D.MipSlice = 0;
			rtDesc.Texture2D.PlaneSlice = 0;

			mDevice->CreateRenderTargetView(curTexEntry.Resource, &rtDesc, mRenderTargetHeap->GetCPU());
			curTexEntry.RenderTarget = mRenderTargetHeap->GetCPU();
			mRenderTargetHeap->OffsetHandles(1);
		}
		// Depth stencil views
		if ((flags & TextureFlags::DepthStencil) == TextureFlags::DepthStencil)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC depthDesc = {};
			depthDesc.Format = ToDXGIFormat(format);
			depthDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			depthDesc.Flags = D3D12_DSV_FLAG_NONE;
			depthDesc.Texture2D.MipSlice = 0;

			mDevice->CreateDepthStencilView(curTexEntry.Resource, &depthDesc, mDepthStencilHeap->GetCPU());
			curTexEntry.DepthStencil = mDepthStencilHeap->GetCPU();
			mDepthStencilHeap->OffsetHandles(1);
		}
		// Full view
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc2D = {};
			desc2D.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc2D.Format = ToDXGIFormat(format);
			if (format == Format::Depth24_Stencil8)
			{
				desc2D.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			}
			desc2D.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc2D.Texture2D.MipLevels = mips;
			desc2D.Texture2D.MostDetailedMip = 0;
			desc2D.Texture2D.PlaneSlice = 0;
			desc2D.Texture2D.ResourceMinLODClamp = 0.0f;

			curTexEntry.FullView = mViewsHeap.GetCPU();
			mDevice->CreateShaderResourceView(curTexEntry.Resource, &desc2D, curTexEntry.FullView);
			mViewsHeap.OffsetHandles(1);
		}
		// Per mip-view
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc2D = {};
			desc2D.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc2D.Format = ToDXGIFormat(format);
			if (format == Format::Depth24_Stencil8)
			{
				desc2D.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			}
			desc2D.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc2D.Texture2D.MipLevels = 1;
			desc2D.Texture2D.PlaneSlice = 0;
			desc2D.Texture2D.ResourceMinLODClamp = 0.0f;

			curTexEntry.MipViews = new D3D12_CPU_DESCRIPTOR_HANDLE[mips];
			for (uint32_t m = 0; m < mips; m++)
			{
				desc2D.Texture2D.MostDetailedMip = m;
				curTexEntry.MipViews[m] = mViewsHeap.GetCPU();
				mDevice->CreateShaderResourceView(curTexEntry.Resource, &desc2D, curTexEntry.MipViews[m]);
				mViewsHeap.OffsetHandles(1);
			}
		}
		// RW Views
		if (isUav)
		{
			// Full view (no such thing as full view for rw)
			{
			}
			// Per mip-view
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC desc2dRW = {};
				desc2dRW.Format = ToDXGIFormat(format);
				desc2dRW.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				desc2dRW.Texture2D.PlaneSlice = 0;

				curTexEntry.MipViewsRW = new D3D12_CPU_DESCRIPTOR_HANDLE[mips];
				for (uint32_t m = 0; m < mips; m++)
				{
					desc2dRW.Texture2D.MipSlice = m;
					curTexEntry.MipViewsRW[m] = mViewsHeap.GetCPU();
					mDevice->CreateUnorderedAccessView(curTexEntry.Resource,nullptr, &desc2dRW, curTexEntry.MipViewsRW[m]);
					mViewsHeap.OffsetHandles(1);
				}
			}
		}

		return handle;
	}

	TextureHandle DX12GraphicsInterface::CreateTextureCube(uint32_t size, uint32_t mips, uint32_t layers, Format format, TextureFlags::T flags, void * data)
	{
		if ((size * layers * mips) == 0)
		{
			return InvalidTexture;
		}

		TextureHandle handle;
		TextureEntry& curTexEntry = mTexturesPool.GetFreeEntry(handle.Handle);

		bool isUav = false;
		bool isCubeArray = layers > 1;

		D3D12_RESOURCE_FLAGS dxflags = D3D12_RESOURCE_FLAG_NONE;
		if ((flags & TextureFlags::UnorderedAccess) == TextureFlags::UnorderedAccess)
		{
			isUav = true;
			dxflags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}
		if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget)
		{
			dxflags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}
		if ((flags & TextureFlags::DepthStencil) == TextureFlags::DepthStencil)
		{
			dxflags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		}
		CD3DX12_RESOURCE_DESC texDesc;
		texDesc = CD3DX12_RESOURCE_DESC::Tex2D
		(
			ToDXGIFormatTypeless(format),
			size,
			size,
			layers * 6,
			mips,
			1, 0,
			dxflags
		);
		CD3DX12_HEAP_PROPERTIES heapP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto& state = curTexEntry.State;
		state = isUav ? UNORDERED_ACCESS : SRV_READ;
		mDevice->CreateCommittedResource
		(
			&heapP, D3D12_HEAP_FLAG_NONE,
			&texDesc, state,
			nullptr,
			IID_PPV_ARGS(&curTexEntry.Resource)
		);

		// Here we cache a few values that we will use to upload the mips:
		UINT64 totalSize;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* footPrints = new D3D12_PLACED_SUBRESOURCE_FOOTPRINT[mips];
		UINT64* rowSizes = new UINT64[mips];
		mDevice->GetCopyableFootprints(&texDesc, 0, mips, 0, footPrints, nullptr, rowSizes, &totalSize);

		// Upload buffer
		// Sometimes the following CreateCommittedResource call will fail with "wrong memory preference..." 
		// setting again fixes it. Prob because it is a static member¿??¿
		heapP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		mDevice->CreateCommittedResource
		(
			&heapP, D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(totalSize), D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&curTexEntry.UploadHeap)
		);
		if (data)
		{
			// Implement this..
			assert(false);
			auto baseDesc = curTexEntry.Resource->GetDesc();
			uint64_t curOff = 0;
			for (uint32_t i = 0; i < mips; i++)
			{
				const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& curPrint = footPrints[i];

				UINT64 curSliceSize = rowSizes[i] * curPrint.Footprint.Height;

				D3D12_SUBRESOURCE_DATA d = {};
				d.pData = (void*)(curOff + (unsigned char*)data);
				d.RowPitch = rowSizes[i];
				d.SlicePitch = curSliceSize;

				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(curTexEntry.Resource, state, COPY_DST));
				UpdateSubresources(mDefaultSurface.CmdContext, curTexEntry.Resource, curTexEntry.UploadHeap, curPrint.Offset, i, 1, &d);
				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(curTexEntry.Resource, COPY_DST, state));

				curOff += rowSizes[i] * curPrint.Footprint.Height;
			}
		}

		delete[] footPrints;
		delete[] rowSizes;

		// Render target views
		if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget)
		{
			// Implement this
			assert(false);
			//D3D12_RENDER_TARGET_VIEW_DESC rtDesc = {};
			//rtDesc.Format = ToDXGIFormat(format);
			//rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			//rtDesc.Texture2D.MipSlice = 0;
			//rtDesc.Texture2D.PlaneSlice = 0;
			//
			//mDevice->CreateRenderTargetView(curTexEntry.Resource, &rtDesc, mRenderTargetHeap->GetCPU());
			//curTexEntry.RenderTarget = mRenderTargetHeap->GetCPU();
			//mRenderTargetHeap->OffsetHandles(1);
		}
		// Depth stencil views
		if ((flags & TextureFlags::DepthStencil) == TextureFlags::DepthStencil)
		{
			// implement this
			assert(false);
			//D3D12_DEPTH_STENCIL_VIEW_DESC depthDesc = {};
			//depthDesc.Format = ToDXGIFormat(format);
			//depthDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			//depthDesc.Flags = D3D12_DSV_FLAG_NONE;
			//depthDesc.Texture2D.MipSlice = 0;
			//
			//mDevice->CreateDepthStencilView(curTexEntry.Resource, &depthDesc, mDepthStencilHeap->GetCPU());
			//curTexEntry.DepthStencil = mDepthStencilHeap->GetCPU();
			//mDepthStencilHeap->OffsetHandles(1);
		}
		// Full view
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC descCube = {};
			descCube.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			descCube.Format = ToDXGIFormat(format);
			if (format == Format::Depth24_Stencil8)
			{
				descCube.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			}
			descCube.ViewDimension = isCubeArray ? D3D12_SRV_DIMENSION_TEXTURECUBEARRAY : D3D12_SRV_DIMENSION_TEXTURECUBE;
			if (isCubeArray)
			{
				descCube.TextureCubeArray.MipLevels = mips;
				descCube.TextureCubeArray.MostDetailedMip = 0;
				descCube.TextureCubeArray.ResourceMinLODClamp = 0.0f;
				descCube.TextureCubeArray.First2DArrayFace = 0;
				descCube.TextureCubeArray.NumCubes = layers;
			}
			else
			{
				descCube.TextureCube.MipLevels = mips;
				descCube.TextureCube.MostDetailedMip = 0;
				descCube.TextureCube.ResourceMinLODClamp = 0.0f;
			}

			curTexEntry.FullView = mViewsHeap.GetCPU();
			mDevice->CreateShaderResourceView(curTexEntry.Resource, &descCube, curTexEntry.FullView);
			mViewsHeap.OffsetHandles(1);
		}
		// Per mip-view
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC descCube = {};
			descCube.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			descCube.Format = ToDXGIFormat(format);
			if (format == Format::Depth24_Stencil8)
			{
				descCube.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			}
			descCube.ViewDimension = isCubeArray ? D3D12_SRV_DIMENSION_TEXTURECUBEARRAY : D3D12_SRV_DIMENSION_TEXTURECUBE;
			if (isCubeArray)
			{
				descCube.TextureCubeArray.MipLevels = 1;
				descCube.TextureCubeArray.ResourceMinLODClamp = 0.0f;
				descCube.TextureCubeArray.First2DArrayFace = 0;
				descCube.TextureCubeArray.NumCubes = layers;
			}
			else
			{
				descCube.TextureCube.MipLevels = 1;
				descCube.TextureCube.ResourceMinLODClamp = 0.0f;
			}

			curTexEntry.MipViews = new D3D12_CPU_DESCRIPTOR_HANDLE[mips];
			for (uint32_t m = 0; m < mips; m++)
			{
				if (isCubeArray)
				{
					descCube.TextureCubeArray.MostDetailedMip = m;
				}
				else
				{
					descCube.TextureCubeArray.MostDetailedMip = m;
				}
				curTexEntry.MipViews[m] = mViewsHeap.GetCPU();
				mDevice->CreateShaderResourceView(curTexEntry.Resource, &descCube, curTexEntry.MipViews[m]);
				mViewsHeap.OffsetHandles(1);
			}
		}
		// RW Views
		if (isUav)
		{
			// Per mip-view
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC descCubeRW = {};
				descCubeRW.Format = ToDXGIFormat(format);
				descCubeRW.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				descCubeRW.Texture2DArray.PlaneSlice = 0;
				descCubeRW.Texture2DArray.ArraySize = layers * 6;
				descCubeRW.Texture2DArray.FirstArraySlice = 0;

				curTexEntry.MipViewsRW = new D3D12_CPU_DESCRIPTOR_HANDLE[mips];
				for (uint32_t m = 0; m < mips; m++)
				{
					descCubeRW.Texture2DArray.MipSlice = m;
					curTexEntry.MipViewsRW[m] = mViewsHeap.GetCPU();
					mDevice->CreateUnorderedAccessView(curTexEntry.Resource, nullptr, &descCubeRW, curTexEntry.MipViewsRW[m]);
					mViewsHeap.OffsetHandles(1);
				}
			}
		}

		return handle;
	}

	TextureHandle DX12GraphicsInterface::CreateTexture3D(uint32_t width, uint32_t height, uint32_t mips, uint32_t layers, Format format, TextureFlags::T flags /* = TextureFlagNone*/, void* data /*= nullptr*/)
	{
		if ((width * height * layers * mips) == 0)
		{
			return InvalidTexture;
		}

		TextureHandle handle;
		TextureEntry& curTexEntry = mTexturesPool.GetFreeEntry(handle.Handle);
		bool isUav = false;

		D3D12_RESOURCE_FLAGS dxflags = D3D12_RESOURCE_FLAG_NONE;
		if ((flags & TextureFlags::UnorderedAccess) == TextureFlags::UnorderedAccess)
		{
			isUav = true;
			dxflags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}
		if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget)
		{
			assert(false); // This is actually supported, not implemented tho
			dxflags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}
		if ((flags & TextureFlags::DepthStencil) == TextureFlags::DepthStencil)
		{
			assert(false); // This is not supported
			dxflags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		}
		CD3DX12_RESOURCE_DESC texDesc;
		texDesc = CD3DX12_RESOURCE_DESC::Tex3D
		(
			ToDXGIFormatTypeless(format),
			width,
			height,
			layers,
			mips,
			dxflags
		);
		CD3DX12_HEAP_PROPERTIES heapP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto& state = curTexEntry.State;
		state = SRV_READ;
		mDevice->CreateCommittedResource
		(
			&heapP, D3D12_HEAP_FLAG_NONE,
			&texDesc, state,
			nullptr,
			IID_PPV_ARGS(&curTexEntry.Resource)
		);

		// Here we cache a few values that we will use to upload the mips:
		UINT64 totalSize;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* footPrints = new D3D12_PLACED_SUBRESOURCE_FOOTPRINT[mips];
		UINT64* rowSizes = new UINT64[mips];
		mDevice->GetCopyableFootprints(&texDesc, 0, mips, 0, footPrints, nullptr, rowSizes, &totalSize);

		// Upload buffer
		// Sometimes the following CreateCommittedResource call will fail with "wrong memory preference..." 
		// setting again fixes it. Prob because it is a static member¿??¿
		heapP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		mDevice->CreateCommittedResource
		(
			&heapP, D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(totalSize), D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&curTexEntry.UploadHeap)
		);
		if (data)
		{
			auto baseDesc = curTexEntry.Resource->GetDesc();
			uint64_t curOff = 0;
			for (uint32_t i = 0; i < mips; i++)
			{
				const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& curPrint = footPrints[i];

				UINT64 curSliceSize = rowSizes[i] * curPrint.Footprint.Height;

				D3D12_SUBRESOURCE_DATA d = {};
				d.pData = (void*)(curOff + (unsigned char*)data);
				d.RowPitch = rowSizes[i];
				d.SlicePitch = curSliceSize;

				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(curTexEntry.Resource, state, COPY_DST));
				UpdateSubresources(mDefaultSurface.CmdContext, curTexEntry.Resource, curTexEntry.UploadHeap, curPrint.Offset, i, 1, &d);
				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(curTexEntry.Resource, COPY_DST, state));

				curOff += rowSizes[i] * curPrint.Footprint.Height;
			}
		}

		delete[] footPrints;
		delete[] rowSizes;

		// Render target view (can we have 3D rts???)
		if ((flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget)
		{
		}
		// Depthstencil view (can we have 3d depths??)
		if ((flags & TextureFlags::DepthStencil) == TextureFlags::DepthStencil)
		{
		}
		// Full view
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc3D = {};
			desc3D.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc3D.Format = ToDXGIFormat(format);
			desc3D.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			desc3D.Texture3D.MipLevels = mips;
			desc3D.Texture3D.MostDetailedMip = 0;
			desc3D.Texture3D.ResourceMinLODClamp = 0.0f;

			curTexEntry.FullView = mViewsHeap.GetCPU();
			mDevice->CreateShaderResourceView(curTexEntry.Resource, &desc3D, curTexEntry.FullView);
			mViewsHeap.OffsetHandles(1);
		}
		// Per mip-view
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc3D = {};
			desc3D.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc3D.Format = ToDXGIFormat(format);
			desc3D.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			desc3D.Texture3D.MipLevels = 1;
			desc3D.Texture3D.MostDetailedMip = 0;
			desc3D.Texture3D.ResourceMinLODClamp = 0.0f;

			curTexEntry.MipViews = new D3D12_CPU_DESCRIPTOR_HANDLE[mips];
			for (uint32_t m = 0; m < mips; m++)
			{
				desc3D.Texture3D.MostDetailedMip = m;
				curTexEntry.MipViews[m] = mViewsHeap.GetCPU();
				mDevice->CreateShaderResourceView(curTexEntry.Resource, &desc3D, curTexEntry.MipViews[m]);
				mViewsHeap.OffsetHandles(1);
			}
		}
		// RW Views
		if (isUav)
		{
			// Full view (no such thing as full view for rw)
			{
			}
			// Per mip-view
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC desc3dRW = {};
				desc3dRW.Format = ToDXGIFormat(format);
				desc3dRW.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
				desc3dRW.Texture3D.FirstWSlice = 0;
				desc3dRW.Texture3D.WSize = layers;

				curTexEntry.MipViewsRW = new D3D12_CPU_DESCRIPTOR_HANDLE[mips];
				for (uint32_t m = 0; m < mips; m++)
				{
					desc3dRW.Texture3D.MipSlice = m;
					curTexEntry.MipViewsRW[m] = mViewsHeap.GetCPU();
					mDevice->CreateUnorderedAccessView(curTexEntry.Resource, nullptr, &desc3dRW, curTexEntry.MipViewsRW[m]);
					mViewsHeap.OffsetHandles(1);

					desc3dRW.Texture3D.WSize = desc3dRW.Texture3D.WSize / 2;
				}
			}
			// Per layer view...
		}
		return handle;
	}

	GPUQueryHandle DX12GraphicsInterface::CreateQuery(const GPUQueryType& type)
	{
		GPUQueryHandle handle;
		switch (type)
		{
			case GPUQueryType::Timestamp:
			{
				QueryEntry& entry = mTimeStampQueriesPool.GetFreeEntry(handle.Handle);
				entry.Type = type;
				entry.HeapIdx = handle.Handle;
				for (int i = 0; i < NUM_BACK_BUFFERS; i++)
				{
					entry.FenceValues[i] = 0;
					mDevice->CreateFence(entry.FenceValues[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&entry.Fences[i]));
				}
				break;
			}
			default:break;
		}

		return handle;
	}

	GraphicsPipeline DX12GraphicsInterface::CreateGraphicsPipeline(const GraphicsPipelineDescription& desc)
	{
		GraphicsPipeline handle;
		GraphicsPipelineEntry& entry = mGraphicsPipelinesPool.GetFreeEntry(handle.Handle);
		CreatePSO(desc,entry);		
		
		// Cache it so we can recompile at runtime
		entry.Desc = desc;
		entry.Desc.VertexShader = desc.VertexShader;
		entry.Desc.PixelShader = desc.PixelShader;
		entry.Desc.VertexDescription = VertexInputDescription(desc.VertexDescription);

		return handle;
	}

	ComputePipeline DX12GraphicsInterface::CreateComputePipeline(const ComputePipelineDescription& desc)
	{
		ComputePipeline handle;
		ComputePipelineEntry& entry = mComputePipelinesPool.GetFreeEntry(handle.Handle);
		CreatePSO(desc, entry);

		// Cache it
		entry.Desc = desc;

		return handle;
	}

	void DX12GraphicsInterface::ReloadGraphicsPipeline(GraphicsPipeline& pipeline)
	{
		// Start by releasing the old entry:
		GraphicsPipelineEntry& entry = mGraphicsPipelinesPool.GetEntry(pipeline.Handle);
		mReleaseManager.ReleaseItem(entry.Pso);
		entry.Pso = nullptr;

		// Now lets create it again:
		CreatePSO(entry.Desc, entry);
	}

	void DX12GraphicsInterface::ReloadComputePipeline(ComputePipeline & pipeline)
	{
		// TO-DO
	}

	void DX12GraphicsInterface::ReleaseTexture(TextureHandle& handle)
	{
		if (handle.Handle == InvalidTexture.Handle)
		{
			assert(false);
			return;
		}
		mReleaseManager.ReleaseItem(mTexturesPool.GetEntry(handle.Handle).Resource);

		handle.Handle = InvalidTexture.Handle;
	}

	void DX12GraphicsInterface::ReleaseGraphicsPipeline(GraphicsPipeline& pipeline)
	{
		if (pipeline.Handle == InvalidGraphicsPipeline.Handle)
		{
			assert(false);
			return;
		}
		
		// Release the pipeline:
		GraphicsPipelineEntry& graphicsPipeline = mGraphicsPipelinesPool.GetEntry(pipeline.Handle);
		mReleaseManager.ReleaseItem(graphicsPipeline.Pso);

		// Remove from the pool:
		mGraphicsPipelinesPool.RemoveEntry(pipeline.Handle);

		// Reset data
		pipeline.Handle = InvalidGraphicsPipeline.Handle;
	}

	void DX12GraphicsInterface::ReleaseComputePipeline(ComputePipeline& pipeline)
	{
	}

	void DX12GraphicsInterface::ReleaseBuffer(BufferHandle& buffer)
	{

	}

	void DX12GraphicsInterface::SetBufferData(const BufferHandle& buffer, int size, int offset, void* data)
	{
		BufferEntry& bufferEntry = mBuffersPool.GetEntry(buffer.Handle);
		if (bufferEntry.Type == BufferType::ConstantBuffer)
		{
			std::cout << "Do not call SetBufferData on a Constant buffer, just use the SetConstantBuffer method. \n";
			return;
		}
		auto dstDesc = bufferEntry.Buffer->GetDesc();
		if (buffer.Handle < MAX_BUFFERS && buffer.Handle != InvalidBuffer.Handle && bufferEntry.Buffer != nullptr && size <= dstDesc.Width)
		{
			D3D12_SUBRESOURCE_DATA sdata = {};
			sdata.pData	= data;
			sdata.RowPitch = size;
			sdata.SlicePitch = 0;
			auto& state = bufferEntry.State;
			bool changed = false;
			if (state != (COPY_DST))
			{
				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(bufferEntry.Buffer, state, COPY_DST));
				changed = true;
			}
			UpdateSubresources(mDefaultSurface.CmdContext, bufferEntry.Buffer, bufferEntry.UploadHeap, offset, 0, 1, &sdata);
			if (changed)
			{
				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(bufferEntry.Buffer, COPY_DST, state));
			}
		}
		else
		{
			std::cout << "Invalid buffer ! \n";
		}
	}

	void DX12GraphicsInterface::SetVertexBuffer(const BufferHandle& buffer,int size, int eleSize)
	{
		BufferEntry& bufferEntry = mBuffersPool.GetEntry(buffer.Handle);
		if (buffer.Handle < MAX_BUFFERS && buffer.Handle != InvalidBuffer.Handle && bufferEntry.Buffer != nullptr)
		{
			D3D12_VERTEX_BUFFER_VIEW view	= {};
			view.BufferLocation				= bufferEntry.Buffer->GetGPUVirtualAddress();
			view.SizeInBytes				= size;
			view.StrideInBytes				= eleSize;
			mDefaultSurface.CmdContext->IASetVertexBuffers(0, 1, &view);
		}
	}

	void DX12GraphicsInterface::SetIndexBuffer(const BufferHandle& buffer,int size, Format idxFormat)
	{
		BufferEntry& bufferEntry = mBuffersPool.GetEntry(buffer.Handle);
		if (buffer.Handle < MAX_BUFFERS && buffer.Handle != InvalidBuffer.Handle && bufferEntry.Buffer != nullptr)
		{
			D3D12_INDEX_BUFFER_VIEW view = {};
			view.BufferLocation = bufferEntry.Buffer->GetGPUVirtualAddress();
			view.SizeInBytes = size;
			view.Format = ToDXGIFormat(idxFormat);
			mDefaultSurface.CmdContext->IASetIndexBuffer(&view);
		}
	}

	void DX12GraphicsInterface::SetTopology(const Topology& topology)
	{
		mDefaultSurface.CmdContext->IASetPrimitiveTopology(ToDXGITopology(topology));
	}

	void DX12GraphicsInterface::SetComputePipeline(const ComputePipeline& pipeline)
	{
		mCurrentIsCompute = true;
		const auto& pso = mComputePipelinesPool.GetEntry(pipeline.Handle).Pso;
		
		if (pipeline.Handle != InvalidComputePipeline.Handle && pso != nullptr)
		{
			mDefaultSurface.CmdContext->SetComputeRootSignature(mGraphicsRootSignature);
			mDefaultSurface.CmdContext->SetPipelineState(pso);

			// Setup NULL views tier 1
			// Why is this a compute only thing??
			int idx = mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();
			auto startSlot = mFrameHeap[idx]->GetCPU();
			for (int i = 0; i < NUM_CBVS; i++)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE curSlot = {};
				CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(curSlot, startSlot, i *  mFrameHeap[idx]->GetIncrementSize());
				mDevice->CopyDescriptorsSimple(1, curSlot, mNullCbv, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
			for (int i = 0; i < NUM_SRVS; i++)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE curSlot = {};
				CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(curSlot, startSlot, (i + NUM_CBVS) *  mFrameHeap[idx]->GetIncrementSize());
				mDevice->CopyDescriptorsSimple(1, curSlot, mNullSrv, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
			for (int i = 0; i < NUM_UAVS; i++)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE curSlot = {};
				CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(curSlot, startSlot, (i + NUM_CBVS + NUM_SRVS) *  mFrameHeap[idx]->GetIncrementSize());
				mDevice->CopyDescriptorsSimple(1, curSlot, mNullUav	, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
		}
	}

	void DX12GraphicsInterface::SetGraphicsPipeline(const GraphicsPipeline& pipeline)
	{
		mCurrentIsCompute = false;
		GraphicsPipelineEntry& psoEntry = mGraphicsPipelinesPool.GetEntry(pipeline.Handle);
		if (pipeline.Handle != InvalidGraphicsPipeline.Handle && psoEntry.Pso != nullptr)
		{
			mDefaultSurface.CmdContext->SetGraphicsRootSignature(mGraphicsRootSignature);
			mDefaultSurface.CmdContext->SetPipelineState(psoEntry.Pso);
		}
	}

	void DX12GraphicsInterface::Dispatch(int x, int y, int z)
	{
		FlushHeap(false);
		FlushBarriers();

		mDefaultSurface.CmdContext->Dispatch((UINT)x, (UINT)y, (UINT)z);
	}

	void DX12GraphicsInterface::Draw(uint32_t numvtx, uint32_t vtxOffset)
	{
		FlushHeap();
		FlushBarriers();

		mDefaultSurface.CmdContext->DrawInstanced(numvtx, 1, vtxOffset, 0);

		mNumDrawCalls++;
	}

	void DX12GraphicsInterface::DrawIndexed(uint32_t numIdx, uint32_t idxOff /*= 0*/, uint32_t vtxOff /*= 0*/)
	{
		FlushHeap();
		FlushBarriers();

		mDefaultSurface.CmdContext->DrawIndexedInstanced(numIdx, 1, idxOff, vtxOff, 0);

		mNumDrawCalls++;
	}

	void DX12GraphicsInterface::SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float zmin /*=0.0f*/, float zmax/*=1.0f*/)
	{
		D3D12_VIEWPORT vp;
		vp.TopLeftX = (FLOAT)x;
		vp.TopLeftY = (FLOAT)y;
		vp.Width	= (FLOAT)w;
		vp.Height	= (FLOAT)h;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		mDefaultSurface.CmdContext->RSSetViewports(1, &vp);
	}

	void DX12GraphicsInterface::SetScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
	{
		D3D12_RECT s;
		s.left	= (LONG)x;
		s.top	= (LONG)y;
		s.right = (LONG)w;
		s.bottom= (LONG)h;
		mDefaultSurface.CmdContext->RSSetScissorRects(1, &s);
	}

	void DX12GraphicsInterface::SetConstantBuffer(const BufferHandle& buffer, uint8_t slot, uint32_t size, void* data)
	{
		BufferEntry& bufferEntry = mBuffersPool.GetEntry(buffer.Handle);
		if (buffer.Handle < MAX_BUFFERS && buffer.Handle != InvalidBuffer.Handle && bufferEntry.UploadHeap != nullptr)
		{
			if (data)
			{
				if (mFrame != bufferEntry.LastFrame)
				{
					bufferEntry.LastFrame = mFrame;
					bufferEntry.CopyCount = 0;
				}
				const CD3DX12_RANGE  read(0, 0);
				uint8_t* pData = nullptr;
				uint64_t intermediateOffset = 0;
				bufferEntry.UploadHeap->Map(0, &read, reinterpret_cast<void**>(&pData));
				{
					intermediateOffset	+= ((CB_INTERMIDIATE_SIZE) / NUM_BACK_BUFFERS) * mCurBackBuffer;
					intermediateOffset	+= ((size + 255) & ~255) * bufferEntry.CopyCount;
					pData				= pData + intermediateOffset;
					memcpy(pData, data, size);
				}
				bufferEntry.UploadHeap->Unmap(0, nullptr);

				bool changed = false;
				if (bufferEntry.State != (COPY_DST))
				{
					TransitionResource(bufferEntry.Buffer, bufferEntry.State, COPY_DST, true);
					changed = true;
				}
				mDefaultSurface.CmdContext->CopyBufferRegion(bufferEntry.Buffer, 0, bufferEntry.UploadHeap, intermediateOffset, size);
				if (changed)
				{
					TransitionResource(bufferEntry.Buffer, COPY_DST, bufferEntry.State);
				}
				bufferEntry.CopyCount++;
			}

			// Copy the view:
			{
				BindingState::Slot& bindSlot = mBindingState.CBSlots[slot];
				if (bindSlot.Null || (bindSlot.CPUView != bufferEntry.CBV))
				{
					bindSlot.CPUView = bufferEntry.CBV;
					bindSlot.Null = false;
					mBindingState.Dirty = true;
				}
			}
		}
	}

	void DX12GraphicsInterface::SetResource(const TextureHandle& texture, uint8_t slot)
	{
		if (!CHECK_TEXTURE(texture))
		{
			ERR("Trying to set an invalid texture!");
			return;
		}

		UINT idx = mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();
		TextureEntry& entry = mTexturesPool.GetEntry(texture.Handle);
		ID3D12Resource* res = entry.Resource;

		if (entry.State != (SRV_READ))
		{
			TransitionResource(res, entry.State, SRV_READ);
		}
		entry.State = SRV_READ;

		BindingState::Slot& bindSlot = mBindingState.SRSlots[slot];
		if (bindSlot.Null || (bindSlot.CPUView != entry.FullView))
		{
			bindSlot.CPUView = entry.FullView;
			bindSlot.Null = false;
			mBindingState.Dirty = true;
		}
	}

	void DX12GraphicsInterface::SetResource(const BufferHandle& buffer, uint8_t slot)
	{
		if (!CHECK_BUFFER(buffer))
		{
			ERR("Trying to set an invalid buffer!");
			return;
		}

		UINT idx = mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();
		BufferEntry& entry = mBuffersPool.GetEntry(buffer.Handle);
		ID3D12Resource* res = entry.Buffer;

		if (entry.State != (SRV_READ))
		{
			TransitionResource(res, entry.State, SRV_READ);
			entry.State = SRV_READ;
		}

		BindingState::Slot& bindSlot = mBindingState.SRSlots[slot];
		if (bindSlot.Null || (bindSlot.CPUView != entry.GPUBufferView))
		{
			bindSlot.CPUView = entry.GPUBufferView;
			bindSlot.Null = false;
			mBindingState.Dirty = true;
		}
	}

	void DX12GraphicsInterface::SetRWResource(const TextureHandle& texture, uint8_t slot, uint32_t mip)
	{
		if (!CHECK_TEXTURE(texture))
		{
			ERR("Trying to set invalid RWTexture!");
			return;
		}
		UINT idx = mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();
		TextureEntry& entry = mTexturesPool.GetEntry(texture.Handle);
		ID3D12Resource* res = entry.Resource;
		
		if (entry.State != (UNORDERED_ACCESS))
		{
			TransitionResource(res, entry.State, UNORDERED_ACCESS);
			entry.State = UNORDERED_ACCESS;
		}

		BindingState::Slot& bindSlot = mBindingState.UASlots[slot];
		if (bindSlot.Null || (bindSlot.CPUView != entry.MipViewsRW[mip]))
		{
			bindSlot.CPUView = entry.MipViewsRW[mip];
			bindSlot.Null = false;
			mBindingState.Dirty = true;
		}
	}

	void DX12GraphicsInterface::SetRWResource(const BufferHandle& buffer, uint8_t slot)
	{
		if (!CHECK_BUFFER(buffer))
		{
			ERR("Trying to set invalid RWBuffer!");
			return;
		}
		
		UINT idx = mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();
		BufferEntry& entry = mBuffersPool.GetEntry(buffer.Handle);
		ID3D12Resource* res = entry.Buffer;

		if (entry.State != (UNORDERED_ACCESS))
		{
			TransitionResource(res, entry.State, UNORDERED_ACCESS);
			entry.State = UNORDERED_ACCESS;
		}

		BindingState::Slot& bindSlot = mBindingState.UASlots[slot];
		if (bindSlot.Null || (bindSlot.CPUView != entry.GPURWBufferView))
		{
			bindSlot.CPUView = entry.GPURWBufferView;
			bindSlot.Null = false;
			mBindingState.Dirty = true;
		}
	}

	void DX12GraphicsInterface::SetTargets(uint8_t num, TextureHandle* colorTargets, TextureHandle* depth)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE colHandles[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		for (int i = 0; i < num; i++)
		{
			TextureEntry& curTex = mTexturesPool.GetEntry(colorTargets[i].Handle);
			if (curTex.State != D3D12_RESOURCE_STATE_RENDER_TARGET)
			{
				TransitionResource(curTex.Resource, curTex.State, D3D12_RESOURCE_STATE_RENDER_TARGET);
				curTex.State = D3D12_RESOURCE_STATE_RENDER_TARGET;
			}
			colHandles[i] = curTex.RenderTarget;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle = nullptr;
		if (depth)
		{
			TextureEntry& curTex = mTexturesPool.GetEntry(depth->Handle);
			if (curTex.State != D3D12_RESOURCE_STATE_DEPTH_WRITE)
			{
				TransitionResource(curTex.Resource, curTex.State, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				curTex.State = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			}
			depthHandle = &curTex.DepthStencil;
		}
		FlushBarriers();
		mDefaultSurface.CmdContext->OMSetRenderTargets(num, colHandles, false, depthHandle);
	}

	void DX12GraphicsInterface::ClearTargets(uint8_t num, TextureHandle* colorTargets, float clear[4], TextureHandle* depth, float d, uint8_t stencil)
	{
		for (int i = 0; i < num; i++)
		{
			TextureEntry& texEntry = mTexturesPool.GetEntry(colorTargets[i].Handle);
			mDefaultSurface.CmdContext->ClearRenderTargetView(texEntry.RenderTarget, clear, 0, nullptr);
		}
		if (depth)
		{
			TextureEntry& texEntry = mTexturesPool.GetEntry(depth->Handle);
			D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
			mDefaultSurface.CmdContext->ClearDepthStencilView(texEntry.DepthStencil, flags, d, stencil, 0, nullptr);
		}
	}

	void DX12GraphicsInterface::DisableAllTargets()
	{
		UINT idx = mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();
		mDefaultSurface.CmdContext->OMSetRenderTargets(1, &mDefaultSurface.RenderTargets[idx], false, nullptr);
	}

	Format DX12GraphicsInterface::GetOutputFormat()
	{
		return mOutputFormat;
	}

	bool DX12GraphicsInterface::MapBuffer(BufferHandle buffer, unsigned char** outPtr, bool writeOnly /*=true*/)
	{
		D3D12_RANGE range = CD3DX12_RANGE(0, 0);
		BufferEntry& bufferEntry = mBuffersPool.GetEntry(buffer.Handle);
		if ((bufferEntry.CPUAccessMode != CPUAccess::Write && bufferEntry.CPUAccessMode != CPUAccess::ReadWrite))
		{
			outPtr = nullptr;
			assert(false);
			return false;
		}
		if (bufferEntry.UploadHeap)
		{
			HRESULT res = S_OK;
			res = bufferEntry.UploadHeap->Map(0, writeOnly ? &range : nullptr, (void**)outPtr);
			if (FAILED(res))
			{
				outPtr = nullptr;
				return false;
			}
			// Return the current portion of the upload buffer
			size_t off = (mFrame % NUM_BACK_BUFFERS) * bufferEntry.Buffer->GetDesc().Width;
			*outPtr += off;
			bufferEntry.LastFrame = mFrame;
			return true;
		}
		return false;
	}

	void DX12GraphicsInterface::UnMapBuffer(BufferHandle buffer, bool writeOnly/*=true*/)
	{
		// TO-DO: What if the user doesn't unmap.
		BufferEntry& bufferEntry = mBuffersPool.GetEntry(buffer.Handle);
		if (bufferEntry.UploadHeap)
		{
			HRESULT res = S_OK;
			bufferEntry.UploadHeap->Unmap(0, nullptr);
			size_t off = (bufferEntry.LastFrame % NUM_BACK_BUFFERS) * bufferEntry.Buffer->GetDesc().Width;
			bufferEntry.LastFrame = 0;
			// Lets make sure the data is ready to be used by the GPU:
			bool changed = false;
			if (bufferEntry.State != COPY_DST)
			{
				TransitionResource(bufferEntry.Buffer, bufferEntry.State, COPY_DST, true);
				changed = true;
			}			
			mDefaultSurface.CmdContext->CopyBufferRegion(bufferEntry.Buffer, 0, bufferEntry.UploadHeap, off, bufferEntry.UploadHeap->GetDesc().Width / NUM_BACK_BUFFERS);
			if (changed)
			{
				TransitionResource(bufferEntry.Buffer, COPY_DST, bufferEntry.State);
			}
		}
	}

	void DX12GraphicsInterface::SetBlendFactors(float blend[4])
	{
		mDefaultSurface.CmdContext->OMSetBlendFactor(blend);
	}

	glm::u32vec2 DX12GraphicsInterface::GetCurrentRenderingSize()
	{
		return glm::u32vec2(mDefaultSurface.Window->GetWidth(), mDefaultSurface.Window->GetHeight());
	}

	void DX12GraphicsInterface::BeginQuery(const GPUQueryHandle& query, const GPUQueryType& type)
	{
		switch (type)
		{
			case GPUQueryType::Timestamp:
			{
				// Must be end/end. fuck u dx12 api
				assert(false);
				break;
			}
			default:
				break;
		}
	}

	void DX12GraphicsInterface::EndQuery(const GPUQueryHandle& query, const GPUQueryType& type)
	{
		switch (type)
		{
			case GPUQueryType::Timestamp:
			{
				QueryEntry& entry = mTimeStampQueriesPool.GetEntry(query.Handle);
				mDefaultSurface.CmdContext->EndQuery(mTimeStampsHeap, D3D12_QUERY_TYPE_TIMESTAMP, (UINT)query.Handle);
				mDefaultSurface.Queue->Signal(entry.Fences[mCurBackBuffer], ++entry.FenceValues[mCurBackBuffer]);
				break;
			}
			default:
				break;
		}
	}

	ViewHandle DX12GraphicsInterface::Create2DView(TextureHandle resource, int firstMip, int numMips, bool rw /*=false*/)
	{
		// to-do msaa
		if (resource.Handle == InvalidTexture.Handle)
		{
			assert(false);
		}

		TextureEntry& texture = mTexturesPool.GetEntry(resource.Handle);
		D3D12_CPU_DESCRIPTOR_HANDLE heapHandle =  mViewsHeap.GetCPU();
		mViewsHeap.OffsetHandles(1);
		ViewHandle handle = {};
		D3D12_RESOURCE_DESC textureDesc = texture.Resource->GetDesc();

		if (rw)
		{
			assert(numMips == 1);
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			desc.Format = textureDesc.Format;
			desc.Texture2D.MipSlice = firstMip;
			desc.Texture2D.PlaneSlice = 0;
			mDevice->CreateUnorderedAccessView(texture.Resource, nullptr, &desc, heapHandle);
		}
		else
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Format = textureDesc.Format;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; 
			desc.Texture2D.MostDetailedMip = firstMip;
			desc.Texture2D.MipLevels = numMips;
			desc.Texture2D.ResourceMinLODClamp = 0.0f;
			mDevice->CreateShaderResourceView(texture.Resource, &desc, heapHandle);
		}

		// Ahmmmm yeah...
		// maybe use the offset to the heap instead of this shiet
		handle.Handle = heapHandle.ptr;
		return handle;
	}

	ViewHandle DX12GraphicsInterface::Create3DView(TextureHandle resource, int firstMip, int numMips, int firstSlice, int numSlices, bool rw)
	{
		if (resource.Handle == InvalidTexture.Handle)
		{
			assert(false);
		}

		TextureEntry& texture = mTexturesPool.GetEntry(resource.Handle);
		D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = mViewsHeap.GetCPU();
		mViewsHeap.OffsetHandles(1);
		ViewHandle handle = {};
		D3D12_RESOURCE_DESC textureDesc = texture.Resource->GetDesc();

		if (rw)
		{
			assert(numMips == 1);
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			desc.Format = textureDesc.Format;
			desc.Texture3D.FirstWSlice = firstSlice;
			desc.Texture3D.MipSlice = firstMip;
			desc.Texture3D.WSize = numSlices;
			mDevice->CreateUnorderedAccessView(texture.Resource, nullptr, &desc, heapHandle);
		}
		else
		{
			assert(numSlices == 1);
			assert(firstSlice == 0);
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Format = textureDesc.Format;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MipLevels = numMips;
			desc.Texture3D.MostDetailedMip = firstMip;
			desc.Texture3D.ResourceMinLODClamp = 0.0f;
			mDevice->CreateShaderResourceView(texture.Resource, &desc, heapHandle);
		}

		// Ahmmmm yeah...
		// maybe use the offset to the heap instead of this shiet
		handle.Handle = heapHandle.ptr;
		return handle;
	}
}}