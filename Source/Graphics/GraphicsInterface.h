#pragma once

#include <stdint.h>
#include <iostream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include "glm/gtc/matrix_transform.hpp"
#include "glm/glm.hpp"

namespace Graphics
{
	namespace Platform
	{
		class BaseWindow;
	}

	struct ShaderType
	{
		enum T
		{
			Vertex = 1,
			Pixel = 2,
			Compute = 3
		};
	};

	enum BufferType
	{
		VertexBuffer = 0,
		IndexBuffer = 1,
		ConstantBuffer = 2
	};

	struct GPUQueryType
	{
		enum T
		{
			Timestamp
		};
	};

	enum CPUAccess
	{
		Read = 0,
		Write = 1,
		ReadWrite = 2,
		None = 3
	};

	enum Format
	{
		Unknown = 0,
		RGBA_16_Float = 1,
		RG_32_Float = 2,
		RGB_32_Float = 3,
		RGBA_32_Float = 4,
		Depth24_Stencil8 = 5,
		RGBA_8_Unorm = 6,
		RGBA_8_Snorm = 7,
		R_16_Uint = 8,
		R_32_Uint = 9,
		R_8_Unorm = 10,
		R_32_Float = 11,
		R_11_G_11_B_10_Float = 12
	};

	enum Topology
	{
		InvalidTopology = 0,
		TriangleList = 1
	};

	enum TextureFlags
	{
		TextureFlagNone = 0,
		RenderTarget = 1 << 1,
		DepthStencil = 1 << 2,
		UnorderedAccess = 1 << 3
	};

	enum DepthFunc
	{
		Always = 0,
		Never = 1,
		Equal = 2,
		LessEqual = 3,
		GreatEqual = 4
	};

	enum BlendOperation
	{
		BlendOpAdd = 0,
		BlendOpSubstract = 1,
		BlendOpMin = 2,
		BlendOpMax = 3
	};

	enum BlendFunction
	{
		BlendZero = 0,
		BlendOne = 1,
		BlendSrcColor = 2,
		BlendInvSrcColor = 3,
		BlendSrcAlpha = 4,
		BlendInvSrcAlpha = 5,
		BlendDstAlpha = 6,
		BlendInvDstAlpha = 7,
		BlendDstColor = 8,
		BlendInvDstColor = 9,
		BlendFactor = 10
	};

	struct BufferHandle
	{
		uint64_t Handle;
	};

	struct TextureHandle
	{
		uint64_t Handle;
	};

	struct GraphicsPipeline
	{
		uint64_t Handle;
	};

	struct ComputePipeline
	{
		uint64_t Handle;
	};

	struct GPUQueryHandle
	{
		uint64_t Handle;
	};

	struct ViewHandle
	{
		uint64_t Handle;
	};

	struct ShaderDescription
	{
		ShaderDescription()
		{
		}
		ShaderDescription(const ShaderDescription& other)
		{
			Type = other.Type;
			ShaderPath.copy((char*)other.ShaderPath.data(), other.ShaderPath.size());
			ShaderEntryPoint.copy((char*)other.ShaderEntryPoint.data(), other.ShaderEntryPoint.size());
		}
		ShaderType::T Type;
		std::string ShaderPath;
		std::string ShaderEntryPoint;
	};

	struct VertexInputDescription
	{
		VertexInputDescription():
			NeedRelease(false)
		{
		}
		~VertexInputDescription()
		{
			if (NeedRelease && Elements)
			{
				delete[] Elements;
			}
		}
		uint8_t NumElements;
		struct VertexInputElement
		{
			std::string Semantic;
			uint8_t Idx;
			Format EleFormat;
			uint32_t Offset;
		}*Elements;
		bool NeedRelease;
	};

	struct GraphicsPipelineDescription
	{
		GraphicsPipelineDescription()
		{
			DepthEnabled = false;
			DepthWriteEnabled = false;
			memset(ColorFormats, 0, sizeof(ColorFormats));
			memset(&DepthFormat, 0, sizeof(DepthFormat));
		}
		ShaderDescription VertexShader;
		ShaderDescription PixelShader;
		VertexInputDescription VertexDescription;
		bool DepthEnabled;
		bool DepthWriteEnabled;
		DepthFunc DepthFunction;
		Format DepthFormat;
		Format ColorFormats[8];
		struct BlendDesc
		{
			BlendDesc() :Enabled(false),WriteMask(15) { }
			bool Enabled;
			uint8_t WriteMask;
			BlendFunction SrcBlendColor;
			BlendFunction DstBlendColor;
			BlendOperation BlendOpColor;
			BlendFunction SrcBlendAlpha;
			BlendFunction DstBlendAlpha;
			BlendOperation BlendOpAlpha;
		}BlendTargets[8];
	};

	struct ComputePipelineDescription
	{
		ShaderDescription ComputeShader;
	};

	struct BoundingSphere
	{
		BoundingSphere() :Center(0.0f), Radius(-1.0f) {}
		BoundingSphere(glm::vec3 c, float r) :Center(c), Radius(r) {}
		glm::vec3 Center;
		float Radius;
	};

	static const BufferHandle InvalidBuffer = { UINT64_MAX };
	static const TextureHandle InvalidTexture = { UINT64_MAX};
	static const GraphicsPipeline InvalidGraphicsPipeline = { UINT64_MAX };
	static const ComputePipeline InvalidComputePipeline = { UINT64_MAX };

	#define CHECK_TEXTURE(h) (h.Handle != Graphics::InvalidTexture.Handle)

	class GraphicsInterface
	{
	public:
		GraphicsInterface()  {};
		~GraphicsInterface() {};
		virtual bool Initialize(Platform::BaseWindow* ) = 0;
		virtual void StartFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void FlushAndWait() = 0;
		virtual BufferHandle CreateBuffer(BufferType type, CPUAccess cpuAccess, uint64_t size,void* data = nullptr) = 0;
		virtual TextureHandle CreateTexture2D(uint32_t width, uint32_t height, uint32_t mips,uint32_t layers,Format format,TextureFlags flags = TextureFlagNone, void* data = nullptr) = 0;
		virtual TextureHandle CreateTexture3D(uint32_t width, uint32_t height, uint32_t mips, uint32_t layers, Format format, TextureFlags flags = TextureFlagNone, void* data = nullptr) = 0;
		virtual GPUQueryHandle CreateQuery(const GPUQueryType::T& type) = 0;
		virtual GraphicsPipeline CreateGraphicsPipeline(const GraphicsPipelineDescription& desc) = 0;
		virtual ComputePipeline CreateComputePipeline(const ComputePipelineDescription& desc) = 0;
		virtual void ReloadGraphicsPipeline(GraphicsPipeline& pipeline) = 0;
		virtual void ReloadComputePipeline(ComputePipeline& pipeline) = 0;
		virtual void ReleaseTexture(TextureHandle& handle) = 0;
		virtual void ReleaseGraphicsPipeline(GraphicsPipeline& pipeline) = 0;
		virtual void ReleaseComputePipeline(ComputePipeline& pipeline) = 0;
		virtual void ReleaseBuffer(BufferHandle& buffer) = 0;
		virtual void SetBufferData(const BufferHandle& buffer, int size, int offset, void* data) = 0;
		virtual void SetVertexBuffer(const BufferHandle& buffer, int size, int eleSize) = 0;
		virtual void SetIndexBuffer(const BufferHandle& buffer, int size, Format idxFormat) = 0;
		virtual void SetTopology(const Topology& topology) = 0;
		virtual void SetGraphicsPipeline(const GraphicsPipeline& pipeline) = 0;
		virtual void SetComputePipeline(const ComputePipeline& pipeline) = 0;
		virtual void Dispatch(int x, int y, int z) = 0;
		virtual void Draw(uint32_t numvtx,uint32_t vtxOffset) = 0;
		virtual void DrawIndexed(uint32_t numIdx, uint32_t idxOff = 0, uint32_t vtxOff = 0) = 0;
		virtual void SetViewport(float x, float y, float w, float h, float zmin = 0.0f, float zmax = 1.0f) = 0;
		virtual void SetScissor(float x, float y, float w, float h) = 0;
		virtual void SetConstantBuffer(const BufferHandle& buffer, uint8_t slot, uint32_t size, void* data) = 0;
		virtual void SetResource(const TextureHandle& texture, uint8_t slot) = 0;
		virtual void SetRWResource(const TextureHandle& texture, uint8_t slot) = 0;
		virtual void SetTargets(uint8_t num, TextureHandle* colorTargets, TextureHandle* depth) = 0;
		virtual void ClearTargets(uint8_t num, TextureHandle* colorTargets,float clear[4], TextureHandle* depth,float d, uint8_t stencil) = 0;
		virtual void DisableAllTargets() = 0;
		virtual Format GetOutputFormat() = 0;
		virtual bool MapBuffer(BufferHandle buffer, unsigned char** outPtr,bool writeOnly = true) = 0;
		virtual void UnMapBuffer(BufferHandle buffer, bool writeOnly = true) = 0;
		virtual void SetBlendFactors(float blend[4]) {}
		virtual glm::vec2 GetCurrentRenderingSize() = 0;
		virtual void BeginQuery(const GPUQueryHandle& query, const GPUQueryType::T& type) = 0;
		virtual void EndQuery(const GPUQueryHandle& query, const GPUQueryType::T& type) = 0;
		virtual ViewHandle Create2DView(TextureHandle resource, int firstMip, int numMips, bool rw = false) = 0;
		virtual ViewHandle Create3DView(TextureHandle resource, int firstMip, int numMips, int firstSlice, int numSlices, bool rw = false) = 0;
	};
}