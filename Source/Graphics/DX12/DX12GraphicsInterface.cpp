#include "DX12GraphicsInterface.h"
#include "Graphics/Platform/Windows/WWindow.h"
#include <iostream>
#include <vector>

namespace Graphics { namespace DX12 {

	DX12GraphicsInterface::DX12GraphicsInterface():
		mDevice(nullptr),
		mCurBuffer(0),
		mCurTexture(0),
		mGraphicsRootSignature(nullptr),
		mCurGraphicsPipeline(0),
		mFrameHeap(nullptr)
	{
		{
			memset(mBuffers, 0, sizeof(ID3D12Resource*) * MAX_BUFFERS);
			memset(mIntermediateBuffers, 0, sizeof(ID3D12Resource*) * MAX_BUFFERS);
			memset(mBuffersStates, -1, sizeof(D3D12_RESOURCE_STATES) * MAX_BUFFERS);
		}
		{
			memset(mTextures, 0, sizeof(ID3D12Resource*) * MAX_TEXTURES);
			memset(mIntermediateTexture, 0, sizeof(ID3D12Resource*) * MAX_TEXTURES);
			memset(mTexturesStates, 0, sizeof(D3D12_RESOURCE_STATES) * MAX_TEXTURES);
		}
		memset(mGraphicsPipelines, 0, sizeof(ID3D12PipelineState*) * MAX_GRAPHICS_PIPELINES);
	}
	
	DX12GraphicsInterface::~DX12GraphicsInterface()
	{
	}

	bool DX12GraphicsInterface::Initialize(Platform::BaseWindow* targetWindow)
	{
		HWND whandle = (HWND)targetWindow;
		memset(&mDefaultSurface, 0, sizeof(DisplaySurface));
		mDefaultSurface.Window = (Platform::Windows::WWindow*)targetWindow;
		mOutputFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		UINT factoryFlags = 0;
#ifdef _DEBUG
		ID3D12Debug* debugController = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			debugController->Release();
		}
#endif

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
		// Initialize the display surface:
		InitSurface(&mDefaultSurface);

		InitRootSignature();

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
		bufferDesc.Format					= mOutputFormat;
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

		// param 0 (CB0)
		{
			CD3DX12_ROOT_PARAMETER p0;
			p0.InitAsConstantBufferView(0);
			params.push_back(p0);
		}
		// param 1 (CB0)
		{
			CD3DX12_ROOT_PARAMETER p1;
			p1.InitAsConstantBufferView(1);
			params.push_back(p1);
		}
		// param 2 (TEX0)
		{
			CD3DX12_ROOT_PARAMETER p2;
			CD3DX12_DESCRIPTOR_RANGE range;
			range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			p2.InitAsDescriptorTable(1, &range);
			params.push_back(p2);
		}
		// LineaWrapSampler (0)
		{
			CD3DX12_STATIC_SAMPLER_DESC s0;
			s0.Init
			(
				0,
				D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR
			);
			samplers.push_back(s0);
		}
		rsDesc.Init(params.size(), params.data(),samplers.size(),samplers.data());
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ID3D10Blob* rsBlob;
		ID3D10Blob* rsErrorBlob;
		D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &rsErrorBlob);
		mDevice->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&mGraphicsRootSignature));

		// Lets init a heap
		D3D12_DESCRIPTOR_HEAP_DESC hDesc = {};
		hDesc.NodeMask			= 0;
		hDesc.NumDescriptors	= 1024;
		hDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		hDesc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		mFrameHeap = new ID3D12DescriptorHeap*[NUM_BACK_BUFFERS];
		for (int i = 0; i < NUM_BACK_BUFFERS; i++)
		{
			mDevice->CreateDescriptorHeap(&hDesc, IID_PPV_ARGS(&mFrameHeap[i]));
		}

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

		std::string path = "../../Assets/Shaders/HLSL/" + desc.ShaderPath;
		std::wstring wpath(path.begin(), path.end());
		std::string target;
		switch (desc.Type)
		{
		case ShaderType::Vertex:
			target = "vs_5_0"; break;
		case ShaderType::Pixel:
			target = "ps_5_0"; break;
		default:
			target = "none_5_0"; break;
		}
		UINT flags =  D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		if (FAILED(D3DCompileFromFile(wpath.c_str(), nullptr, nullptr, desc.ShaderEntryPoint.c_str(), target.c_str(), flags, 0, &sblob, &error)))
		{
			OutputDebugStringA((char*)error->GetBufferPointer());
			return false;
		}
		outShader.BytecodeLength = sblob->GetBufferSize();
		outShader.pShaderBytecode = sblob->GetBufferPointer();
		return true;
	}

	DXGI_FORMAT DX12GraphicsInterface::ToDXGIFormat(const Graphics::Format& format)
	{
		switch (format)
		{
		case Format::RG_32_Float: return DXGI_FORMAT_R32G32_FLOAT;
		case Format::RGB_32_Float: return DXGI_FORMAT_R32G32B32_FLOAT; 
		case Format::RGBA_32_Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case Format::Unknown:
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY DX12GraphicsInterface::ToDXGITopology(const Graphics::Topology& topology)
	{
		switch (topology)
		{
		case Topology::TriangleList: return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case Topology::InvalidTopology:
		default:
			return D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}

	void DX12GraphicsInterface::StartFrame()
	{
		if (mDefaultSurface.Recording)
		{
			std::cout << "We are recording commands!!!\n";
		}


		// Lets start by making sure it is safe to write to the current buffer
		UINT idx = mDefaultSurface.SwapChain->GetCurrentBackBufferIndex();
		if (mDefaultSurface.GPUFences[idx]->GetCompletedValue() < mDefaultSurface.GPUFencesValues[idx])
		{
			mDefaultSurface.GPUFences[idx]->SetEventOnCompletion(mDefaultSurface.GPUFencesValues[idx], mDefaultSurface.GPUFenceEvent);
			WaitForSingleObject(mDefaultSurface.GPUFenceEvent, INFINITE);
		}
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
		ID3D12DescriptorHeap* heaps[] = { mFrameHeap[idx] };
		context->SetDescriptorHeaps(1, heaps);
		mCPUCurHandle = heaps[0]->GetCPUDescriptorHandleForHeapStart();
		mGPUCurHandle = heaps[0]->GetGPUDescriptorHandleForHeapStart();
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
		mDefaultSurface.SwapChain->Present(1, 0);
		mDefaultSurface.GPUFencesValues[idx]++;
		mDefaultSurface.Queue->Signal(mDefaultSurface.GPUFences[idx], mDefaultSurface.GPUFencesValues[idx]);
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

	BufferHandle DX12GraphicsInterface::CreateBuffer(BufferType type, CPUAccess cpuAccess, uint64_t size, void* data /*= nullptr*/)
	{
		if (type == Graphics::VertexBuffer || type == Graphics::ConstantBuffer)
		{
			D3D12_HEAP_PROPERTIES heapDesc = {};
			auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);
			mDevice->CreateCommittedResource
			(
				&heapProp, 
				D3D12_HEAP_FLAG_NONE, 
				&desc, 
				VERTEX_CB_READ,
				nullptr, 
				IID_PPV_ARGS(&mBuffers[mCurBuffer])
			);
			BufferHandle handle = { mCurBuffer };

			D3D12_RESOURCE_STATES& state = mBuffersStates[handle.Handle];
			state = VERTEX_CB_READ;

			if (data)
			{
				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBuffers[mCurBuffer], state, COPY_DST));
				SetBufferData(handle, size, 0, data);
				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBuffers[mCurBuffer], COPY_DST, state));
			}
			mCurBuffer++;
			return handle;
		}
		return InvalidBuffer;
	}

	TextureHandle DX12GraphicsInterface::CreateTexture2D(uint32_t width, uint32_t height, uint32_t mips, uint32_t layers, Format format, void* data /*= nullptr*/)
	{
		if ((width * height * layers * mips) == 0)
		{
			return InvalidTexture;
		}
		CD3DX12_RESOURCE_DESC texDesc;
		texDesc = CD3DX12_RESOURCE_DESC::Tex2D
		(
			ToDXGIFormat(format),
			width,
			height,
			layers,
			mips,
			1,0,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS		// We probably don´t want to do this by default
		);
		CD3DX12_HEAP_PROPERTIES heapP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto& state = mTexturesStates[mCurTexture];
		state = TEXTURE_READ;
		mDevice->CreateCommittedResource
		(
			&heapP,D3D12_HEAP_FLAG_NONE,
			&texDesc,state,
			nullptr,
			IID_PPV_ARGS(&mTextures[mCurTexture])
		);

		// Upload buffer
		UINT64 rowSize;
		UINT64 totalSize;
		mDevice->GetCopyableFootprints(&texDesc, 0, 1, 0, nullptr, nullptr, &rowSize, &totalSize);
		heapP.Type = D3D12_HEAP_TYPE_UPLOAD;
		mDevice->CreateCommittedResource
		(
			&heapP, D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(totalSize), D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mIntermediateTexture[mCurTexture])
		);
		if (data)
		{
			D3D12_SUBRESOURCE_DATA sdat = {};
			sdat.pData		= data;
			sdat.RowPitch	= rowSize;
			sdat.SlicePitch	= rowSize * height;
			mDefaultSurface.CmdContext->ResourceBarrier(1,&CD3DX12_RESOURCE_BARRIER::Transition(mTextures[mCurTexture], state, COPY_DST));
			UpdateSubresources(mDefaultSurface.CmdContext, mTextures[mCurTexture], mIntermediateTexture[mCurTexture], 0, 0, 1, &sdat);
			mDefaultSurface.CmdContext->ResourceBarrier(1,&CD3DX12_RESOURCE_BARRIER::Transition(mTextures[mCurTexture], COPY_DST,state));
		}
		TextureHandle handle = { mCurTexture };
		mCurTexture++;
		return handle;
	}

	GraphicsPipeline DX12GraphicsInterface::CreateGraphicsPipeline(const GraphicsPipelineDescription& desc)
	{
		// Build the input layout:
		D3D12_INPUT_LAYOUT_DESC inputL = {};
		std::vector<D3D12_INPUT_ELEMENT_DESC> eles;
		eles.resize(desc.VertexDescription.NumElements);
		for (int i = 0; i < desc.VertexDescription.NumElements; i++)
		{
			eles[i].SemanticName		 = desc.VertexDescription.Elements[i].Semantic.c_str();
			eles[i].SemanticIndex		 = desc.VertexDescription.Elements[i].Idx;
			eles[i].Format				 = ToDXGIFormat(desc.VertexDescription.Elements[i].EleFormat);
			eles[i].AlignedByteOffset	 = desc.VertexDescription.Elements[i].Offset;
			eles[i].InputSlotClass		 = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			eles[i].InstanceDataStepRate = 0;
			eles[i].InputSlot			= 0;
		}
		inputL.NumElements = desc.VertexDescription.NumElements;
		inputL.pInputElementDescs = eles.data();

		// Describe the pso
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};		
		LoadShader(desc.PixelShader,psoDesc.PS);
		LoadShader(desc.VertexShader,psoDesc.VS);
		
		psoDesc.BlendState				= CD3DX12_BLEND_DESC::CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		
		psoDesc.DepthStencilState		= CD3DX12_DEPTH_STENCIL_DESC::CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = false;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		psoDesc.RasterizerState			= CD3DX12_RASTERIZER_DESC::CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		
		psoDesc.InputLayout				= inputL;
		psoDesc.pRootSignature			= mGraphicsRootSignature;
		psoDesc.NumRenderTargets		= 1;
		psoDesc.RTVFormats[0]			= mOutputFormat;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; //D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		psoDesc.SampleDesc.Count		= 1;
		psoDesc.SampleDesc.Quality		= 0;
		psoDesc.SampleMask				= 0xffffffff;
		mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mGraphicsPipelines[mCurGraphicsPipeline]));

		GraphicsPipeline handle = { mCurGraphicsPipeline };
		mCurGraphicsPipeline++;

		return handle;
	}

	ComputePipeline DX12GraphicsInterface::CreateComputePipeline(const ComputePipelineDescription& desc)
	{
		return InvalidComputePipeline;
	}

	void DX12GraphicsInterface::SetBufferData(const BufferHandle& buffer, int size, int offset, void* data)
	{
		const auto dst = mBuffers[buffer.Handle];
		auto dstDesc = dst->GetDesc();
		if (buffer.Handle < MAX_BUFFERS && buffer.Handle != InvalidBuffer.Handle && dst != nullptr && size <= dstDesc.Width)
		{
			// Make sure to have an intermediate buffer, if the DST buffers supports
			// CPU read we can just map it! TO-DO!
			if (!mIntermediateBuffers[buffer.Handle])
			{
				auto props = CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				auto desc = CD3DX12_RESOURCE_DESC::Buffer(dstDesc.Width);
				mDevice->CreateCommittedResource
				(
					&props,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&mIntermediateBuffers[buffer.Handle])
				);
			}
			D3D12_SUBRESOURCE_DATA sdata = {};
			sdata.pData		= data;
			sdata.RowPitch	= sdata.SlicePitch = size;
			UpdateSubresources(mDefaultSurface.CmdContext, dst, mIntermediateBuffers[buffer.Handle], 0, 0, 1, &sdata);
		}
		else
		{
			std::cout << "Invalid buffer ! \n";
		}
	}

	void DX12GraphicsInterface::SetVertexBuffer(const BufferHandle& buffer,int size, int eleSize)
	{
		const auto res = mBuffers[buffer.Handle];
		if (buffer.Handle < MAX_BUFFERS && buffer.Handle != InvalidBuffer.Handle && res != nullptr)
		{
			D3D12_VERTEX_BUFFER_VIEW view	= {};
			view.BufferLocation				= res->GetGPUVirtualAddress();
			view.SizeInBytes				= size;
			view.StrideInBytes				= eleSize;
			mDefaultSurface.CmdContext->IASetVertexBuffers(0, 1, &view);
		}
	}

	void DX12GraphicsInterface::SetTopology(const Topology& topology)
	{
		//mDefaultSurface.CmdContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		mDefaultSurface.CmdContext->IASetPrimitiveTopology(ToDXGITopology(topology));
	}

	void DX12GraphicsInterface::SetGraphicsPipeline(const GraphicsPipeline& pipeline)
	{
		auto pso = mGraphicsPipelines[pipeline.Handle];
		if (pipeline.Handle != InvalidGraphicsPipeline.Handle && pso != nullptr)
		{
			mDefaultSurface.CmdContext->SetGraphicsRootSignature(mGraphicsRootSignature);
			mDefaultSurface.CmdContext->SetPipelineState(pso);
		}
	}

	void DX12GraphicsInterface::Draw(uint32_t numvtx, uint32_t vtxOffset)
	{
		mDefaultSurface.CmdContext->SetGraphicsRootDescriptorTable(2, mGPUCurHandle);
		// Offset it for nex draw call
		mGPUCurHandle.Offset(1);
		mCPUCurHandle.Offset(1);

		mDefaultSurface.CmdContext->DrawInstanced(numvtx, 1, vtxOffset, 0);
	}

	void DX12GraphicsInterface::SetViewport(float x, float y, float w, float h, float zmin /*=0.0f*/, float zmax/*=1.0f*/)
	{
		D3D12_VIEWPORT vp;
		vp.TopLeftX = x;
		vp.TopLeftY = y;
		vp.Width	= w;
		vp.Height	= h;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		mDefaultSurface.CmdContext->RSSetViewports(1, &vp);
	}

	void DX12GraphicsInterface::SetScissor(float x, float y, float w, float h)
	{
		D3D12_RECT s;
		s.left	= x;
		s.top	= y;
		s.right = w;
		s.bottom= h;
		mDefaultSurface.CmdContext->RSSetScissorRects(1, &s);
	}

	void DX12GraphicsInterface::SetConstantBuffer(const BufferHandle& buffer, uint8_t slot, uint32_t size, void* data)
	{
		const auto res = mBuffers[buffer.Handle];
		if (buffer.Handle < MAX_BUFFERS && buffer.Handle != InvalidBuffer.Handle && res != nullptr)
		{
			if (data)
			{
				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(res, mBuffersStates[buffer.Handle], COPY_DST));
				SetBufferData(buffer, size, 0, data);
				mDefaultSurface.CmdContext->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(res, COPY_DST, mBuffersStates[buffer.Handle]));
			}
			mDefaultSurface.CmdContext->SetGraphicsRootConstantBufferView(slot, res->GetGPUVirtualAddress());
		}
	}

	void DX12GraphicsInterface::SetTexture(const TextureHandle& texture, uint8_t slot)
	{
		const auto res = mTextures[texture.Handle];
		if (texture.Handle != InvalidTexture.Handle && res != nullptr)
		{
			auto slotHandle = mCPUCurHandle;
			slotHandle.Offset(slot);
			mDevice->CreateShaderResourceView(res, nullptr, slotHandle);
		}
	}
}}