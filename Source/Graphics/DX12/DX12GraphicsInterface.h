#pragma once

#include "Graphics/GraphicsInterface.h"
#include "DX12Common.h"
#include "DX12Heap.h"
#include "DX12ReleaseMananger.h"

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
			State(D3D12_RESOURCE_STATE_VIDEO_DECODE_READ),
			MipViews(nullptr),
			MipViewsRW(nullptr)
		{
		}
		ID3D12Resource* Resource;
		ID3D12Resource* UploadHeap;
		D3D12_RESOURCE_STATES State;
		D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencil;

		D3D12_CPU_DESCRIPTOR_HANDLE FullView;
		D3D12_CPU_DESCRIPTOR_HANDLE* MipViews;
		D3D12_CPU_DESCRIPTOR_HANDLE* MipViewsRW;
	};

	struct QueryEntry
	{
		GPUQueryType::T Type;
		uint32_t HeapIdx; // Points to the 'heap Type' index for this query
		ID3D12Fence* Fences[NUM_BACK_BUFFERS];
		uint64_t FenceValues[NUM_BACK_BUFFERS];
	};

	struct ViewEntry
	{
		D3D12_CPU_DESCRIPTOR_HANDLE View;
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
		BufferType::T Type;
		CPUAccess::T CPUAccessMode;
		GPUAccess::T GPUAccessMode;
		uint64_t LastFrame;
		uint64_t CopyCount; // Used to track how many times we bind a Constant Buffer 
		D3D12_CPU_DESCRIPTOR_HANDLE CBV;
		D3D12_CPU_DESCRIPTOR_HANDLE GPUBufferView;
		D3D12_CPU_DESCRIPTOR_HANDLE GPURWBufferView;
	};
	
	struct GraphicsPipelineEntry
	{
		GraphicsPipelineEntry() :
			Pso(nullptr)
		{
		}
		ID3D12PipelineState* Pso;
		GraphicsPipelineDescription Desc;
	};

	//! Class that manages creation of new resource entries:
	template <class T>
	class ResourcePool
	{
	public:
		ResourcePool<T>() {}
		~ResourcePool<T>() {}
		T& GetFreeEntry(uint64_t& entryIdx)
		{
			entryIdx = 0;
			mEntries.push_back(T());
			entryIdx = mEntries.size() - 1;
			return mEntries.at(entryIdx);
		}
		T& GetEntry(const uint64_t& entryIdx)
		{
			return mEntries.at(entryIdx);
		}

	private:
		std::vector<T> mEntries;
	};

	struct BindingState
	{
		BindingState() :Dirty(true) {};
		struct Slot
		{
			Slot() :Null(true) {};
			CD3DX12_CPU_DESCRIPTOR_HANDLE CPUView;
			bool Null;
		};
		Slot CBSlots[NUM_CBVS];
		Slot SRSlots[NUM_SRVS];
		Slot UASlots[NUM_UAVS];
		bool Dirty;
	};

	class DX12GraphicsInterface : public GraphicsInterface
	{
		friend class DX12ReleaseManager;

	public:
		DX12GraphicsInterface();
		~DX12GraphicsInterface();
		bool Initialize(Platform::BaseWindow* targetWindow)final override;
		void StartFrame()final override;
		void EndFrame()final override;
		void FlushAndWait()final override;
		BufferHandle CreateBuffer(BufferType::T type, CPUAccess::T cpuAccess, GPUAccess::T gpuAccess, uint64_t size, uint32_t stride = 0, void* data = nullptr)final override;
		TextureHandle CreateTexture2D(uint32_t width, uint32_t height, uint32_t mips, uint32_t layers, Format format, TextureFlags flags = TextureFlagNone, void* data = nullptr)final override;
		TextureHandle CreateTexture3D(uint32_t width, uint32_t height, uint32_t mips, uint32_t layers, Format format, TextureFlags flags = TextureFlagNone, void* data = nullptr)final override;
		GPUQueryHandle CreateQuery(const GPUQueryType::T& type)final override;
		GraphicsPipeline CreateGraphicsPipeline(const GraphicsPipelineDescription& desc)final override;
		ComputePipeline CreateComputePipeline(const ComputePipelineDescription& desc)final override;
		void ReloadGraphicsPipeline(GraphicsPipeline& pipeline)final override;
		void ReloadComputePipeline(ComputePipeline& pipeline)final override;
		void ReleaseTexture(TextureHandle& handle)final override;
		void ReleaseGraphicsPipeline(GraphicsPipeline& pipeline)final override;
		void ReleaseComputePipeline(ComputePipeline& pipeline)final override;
		void ReleaseBuffer(BufferHandle& buffer) final override;
		void SetBufferData(const BufferHandle& buffer, int size, int offset, void* data)final override;
		void SetVertexBuffer(const BufferHandle& buffer, int size, int eleSize)final override;
		void SetIndexBuffer(const BufferHandle& buffer,int size, Format idxFormat)final override;
		void SetTopology(const Topology::T& topology)final override;
		void SetComputePipeline(const ComputePipeline& pipeline)final override;
		void SetGraphicsPipeline(const GraphicsPipeline& pipeline)final override;
		void Dispatch(int x, int y, int z)final override;
		void Draw(uint32_t numvtx, uint32_t vtxOffset)final override;
		void DrawIndexed(uint32_t numIdx, uint32_t idxOff = 0,uint32_t vtxOff = 0)final override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float zmin = 0.0f, float zmax = 1.0f)final override;
		void SetScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h)final override;
		void SetConstantBuffer(const BufferHandle& buffer, uint8_t slot, uint32_t size, void* data)final override;
		void SetResource(const TextureHandle& texture, uint8_t slot)final override;
		void SetResource(const BufferHandle& buffer, uint8_t slot)final override;
		void SetRWResource(const TextureHandle& texture, uint8_t slot)final override;
		void SetRWResource(const BufferHandle& buffer, uint8_t slot)final override;
		void SetTargets(uint8_t num, TextureHandle* colorTargets, TextureHandle* depth) final override;
		void ClearTargets(uint8_t num, TextureHandle* colorTargets, float clear[4], TextureHandle* depth, float d, uint8_t stencil)final override;
		void DisableAllTargets()final override;
		Format GetOutputFormat()final override;
		bool MapBuffer(BufferHandle buffer, unsigned char** outPtr, bool writeOnly = true)final override;
		void UnMapBuffer(BufferHandle buffer, bool writeOnly = true)final override;
		void SetBlendFactors(float blend[4])override;;
		glm::vec2 GetCurrentRenderingSize()final override;
		void BeginQuery(const GPUQueryHandle& query, const GPUQueryType::T& type)final override;
		void EndQuery(const GPUQueryHandle& query, const GPUQueryType::T& type)final override;
		ViewHandle Create2DView(TextureHandle resource, int firstMip, int numMips, bool rw = false)final override;
		ViewHandle Create3DView(TextureHandle resource, int firstMip, int numMips, int firstSlice, int numSlices, bool rw = false)final override;

	private:
		void InitSurface(DisplaySurface* surface);
		void InitRootSignature();
		bool LoadShader(const ShaderDescription& desc, D3D12_SHADER_BYTECODE& outShader);
		void FlushHeap(bool graphics = true);
		void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, bool forceFlush = false, uint32_t subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		void FlushBarriers();
		static DXGI_FORMAT ToDXGIFormat(const Graphics::Format& format);
		static DXGI_FORMAT ToDXGIFormatTypeless(const Graphics::Format& format);
		static D3D12_PRIMITIVE_TOPOLOGY ToDXGITopology(const Graphics::Topology::T& topology);
		static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToDXGIPrimitive(const Graphics::Primitive::T& primitive);
		static D3D12_COMPARISON_FUNC ToDX12DepthFunc(const DepthFunc& func);
		static D3D12_BLEND ToDX12Blend(const BlendFunction& func);
		static D3D12_BLEND_OP ToDX12BlendOp(const BlendOperation& op);

		DisplaySurface mDefaultSurface;
		ID3D12Device* mDevice;
		D3D12_FEATURE_DATA_D3D12_OPTIONS mDeviceFeatures;
		Graphics::Format mOutputFormat;

		// Buffer pool
		ResourcePool<BufferEntry> mBuffersPool;

		// Texture pool
		ResourcePool<TextureEntry> mTexturesPool;

		// Query pool
		ResourcePool<QueryEntry> mTimeStampQueriesPool;

		// PSO pools
		GraphicsPipelineEntry mGraphicsPipelines[MAX_GRAPHICS_PIPELINES];
		uint64_t mCurGraphicsPipeline;

		ID3D12PipelineState* mComputePipelines[MAX_COMPUTE_PIPELINES];
		uint64_t mCurComputePipeline;

		// Heaps
		DX12Heap** mFrameHeap;

		// Storage heaps
		DX12Heap* mRenderTargetHeap;
		DX12Heap* mDepthStencilHeap;
		DX12Heap mViewsHeap;
		DX12Heap mNullsHeap;

		D3D12_CPU_DESCRIPTOR_HANDLE mNullCbv;
		D3D12_CPU_DESCRIPTOR_HANDLE mNullUav;
		D3D12_CPU_DESCRIPTOR_HANDLE mNullSrv;

		ID3D12QueryHeap* mTimeStampsHeap;
		ID3D12Resource* mTimeStampsMemory;
		uint8_t* mTimeStampMemPtr;

		ID3D12RootSignature* mGraphicsRootSignature;

		uint64_t mFrame;
		uint64_t mCurBackBuffer;

		uint64_t mNumDrawCalls;

		DX12ReleaseManager mReleaseManager;

		bool mCurrentIsCompute = false;

		std::vector<CD3DX12_RESOURCE_BARRIER> mPendingBarriers;

		BindingState mBindingState;
	};

}
}