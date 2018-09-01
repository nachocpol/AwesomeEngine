#pragma once

#include "Graphics/GraphicsInterface.h"
#include "DX12Common.h"
#include "DX12Heap.h"

namespace Graphics
{
	namespace Platform
	{
		namespace Windows
		{
			class WWindow;
		}
	}
}
namespace Graphics{ namespace DX12
{
	//! This struct holds all the objects needed to
	//! display graphics to a window
	struct DisplaySurface
	{
		IDXGIAdapter1* GPU;
		IDXGISwapChain3* SwapChain;
		ID3D12CommandQueue* Queue;
		
		ID3D12Resource* BackBuffers[NUM_BACK_BUFFERS];
		ID3D12DescriptorHeap* Heap;
		D3D12_CPU_DESCRIPTOR_HANDLE RenderTargets[NUM_BACK_BUFFERS];
		
		Platform::Windows::WWindow* Window;

		ID3D12CommandAllocator* Allocators[NUM_BACK_BUFFERS];
		ID3D12GraphicsCommandList* CmdContext;

		ID3D12Fence* GPUFences[NUM_BACK_BUFFERS];
		UINT64 GPUFencesValues[NUM_BACK_BUFFERS];
		HANDLE GPUFenceEvent;

		bool Recording;
	};

	struct TextureEntry
	{
		TextureEntry() :
			Resource(nullptr),
			UploadHeap(nullptr),
			State(D3D12_RESOURCE_STATE_VIDEO_DECODE_READ)
		{
		}
		ID3D12Resource* Resource;
		ID3D12Resource* UploadHeap;
		D3D12_RESOURCE_STATES State;
		D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencil;
	};

	struct BufferEntry
	{
		BufferEntry() :
			Buffer(nullptr),
			UploadHeap(nullptr),
			LastFrame(0),
			CopyCount(0)
		{
		}
		ID3D12Resource* Buffer;
		ID3D12Resource* UploadHeap;
		D3D12_RESOURCE_STATES State;
		BufferType Type;
		// CB stuff
		uint64_t LastFrame;
		uint64_t CopyCount;
	};
	
	class DX12GraphicsInterface : public GraphicsInterface
	{
	public:
		DX12GraphicsInterface();
		~DX12GraphicsInterface();
		bool Initialize(Platform::BaseWindow* targetWindow)final override;
		void StartFrame()final override;
		void EndFrame()final override;
		void FlushAndWait()final override;
		BufferHandle CreateBuffer(BufferType type, CPUAccess cpuAccess, uint64_t size, void* data = nullptr)final override;
		TextureHandle CreateTexture2D(uint32_t width, uint32_t height, uint32_t mips, uint32_t layers, Format format, TextureFlags flags = TextureFlagNone, void* data = nullptr)final override;
		GraphicsPipeline CreateGraphicsPipeline(const GraphicsPipelineDescription& desc)final override;
		ComputePipeline CreateComputePipeline(const ComputePipelineDescription& desc)final override;
		void SetBufferData(const BufferHandle& buffer, int size, int offset, void* data)final override;
		void SetVertexBuffer(const BufferHandle& buffer, int size, int eleSize)final override;
		void SetIndexBuffer(const BufferHandle& buffer,int size)final override;
		void SetTopology(const Topology& topology)final override;
		void SetGraphicsPipeline(const GraphicsPipeline& pipeline)final override;
		void Draw(uint32_t numvtx, uint32_t vtxOffset)final override;
		void DrawIndexed(uint32_t numIdx)final override;
		void SetViewport(float x, float y, float w, float h, float zmin = 0.0f, float zmax = 1.0f)final override;
		void SetScissor(float x, float y, float w, float h)final override;
		void SetConstantBuffer(const BufferHandle& buffer, uint8_t slot, uint32_t size, void* data)final override;
		void SetTexture(const TextureHandle& texture, uint8_t slot)final override;
		void SetTargets(uint8_t num, TextureHandle* colorTargets, TextureHandle* depth) final override;
		void ClearTargets(uint8_t num, TextureHandle* colorTargets, float clear[4], TextureHandle* depth, float d, uint16_t stencil)final override;
		void DisableAllTargets()final override;
		Format GetOutputFormat()final override;

	private:
		void InitSurface(DisplaySurface* surface);
		void InitRootSignature();
		bool LoadShader(const ShaderDescription& desc, D3D12_SHADER_BYTECODE& outShader);
		static DXGI_FORMAT ToDXGIFormat(const Graphics::Format& format);
		static D3D12_PRIMITIVE_TOPOLOGY ToDXGITopology(const Graphics::Topology& topology);
		static D3D12_COMPARISON_FUNC ToDX12DepthFunc(const DepthFunc& func);
		static D3D12_BLEND ToDX12Blend(const BlendFunction& func);
		static D3D12_BLEND_OP ToDX12BlendOp(const BlendOperation& op);

		DisplaySurface mDefaultSurface;
		ID3D12Device* mDevice;
		Graphics::Format mOutputFormat;

		// Buffer pools
		BufferEntry* mBuffers[MAX_BUFFERS];
		uint64_t mCurBuffer;

		// Texture pool
		TextureEntry* mTextures[MAX_TEXTURES];
		uint64_t mCurTexture;

		// PSO pools
		ID3D12PipelineState* mGraphicsPipelines[MAX_GRAPHICS_PIPELINES];
		uint64_t mCurGraphicsPipeline;

		// Heaps
		DX12Heap** mFrameHeap;

		// Storage heaps
		DX12Heap* mRenderTargetHeap;
		DX12Heap* mDepthStencilHeap;

		ID3D12RootSignature* mGraphicsRootSignature;

		uint64_t mFrame;
		uint64_t mCurBackBuffer;

		uint64_t mNumDrawCalls;
	};

}}