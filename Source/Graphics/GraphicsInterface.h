#pragma once

#include <stdint.h>
#include <iostream>

#include "glm/glm.hpp"

namespace Graphics
{
	namespace Platform
	{
		class BaseWindow;
	}

	enum ShaderType
	{
		Vertex = 1,
		Pixel = 2
	};
	enum BufferType
	{
		VertexBuffer = 0,
		IndexBuffer = 1,
		ConstantBuffer = 2
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
		RG_32_Float = 1,
		RGB_32_Float = 2,
		RGBA_32_Float = 3,
		Depth24_Stencil8 = 4,
		RGBA_8_Unorm = 5
	};
	enum Topology
	{
		InvalidTopology = 0,
		TriangleList = 1
	};
	typedef enum TextureFlags
	{
		TextureFlagNone = 0,
		RenderTarget = 1,
		DepthStencil = 2,
		UnorderedAccess = 3
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

	struct ShaderDescription
	{
		ShaderType Type;
		std::string ShaderPath;
		std::string ShaderEntryPoint;
	};
	struct VertexInputDescription
	{
		uint8_t NumElements;
		struct VertexInputElement
		{
			std::string Semantic;
			uint8_t Idx;
			Format EleFormat;
			uint32_t Offset;
		}*Elements;
	};
	struct GraphicsPipelineDescription
	{
		ShaderDescription VertexShader;
		ShaderDescription PixelShader;
		VertexInputDescription VertexDescription;
	};
	struct ComputePipelineDescription
	{
		std::string ComputeShaderSource;
	};

	static const BufferHandle InvalidBuffer = { UINT64_MAX };
	static const TextureHandle InvalidTexture = { UINT64_MAX};
	static const GraphicsPipeline InvalidGraphicsPipeline = { UINT64_MAX };
	static const ComputePipeline InvalidComputePipeline = { UINT64_MAX };

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
		virtual GraphicsPipeline CreateGraphicsPipeline(const GraphicsPipelineDescription& desc) = 0;
		virtual ComputePipeline CreateComputePipeline(const ComputePipelineDescription& desc) = 0;
		virtual void SetBufferData(const BufferHandle& buffer, int size, int offset, void* data) = 0;
		virtual void SetVertexBuffer(const BufferHandle& buffer, int size, int eleSize) = 0;
		virtual void SetTopology(const Topology& topology) = 0;
		virtual void SetGraphicsPipeline(const GraphicsPipeline& pipeline) = 0;
		virtual void Draw(uint32_t numvtx,uint32_t vtxOffset) = 0;
		virtual void SetViewport(float x, float y, float w, float h, float zmin = 0.0f, float zmax = 1.0f) = 0;
		virtual void SetScissor(float x, float y, float w, float h) = 0;
		virtual void SetConstantBuffer(const BufferHandle& buffer, uint8_t slot, uint32_t size, void* data) = 0;
		virtual void SetTexture(const TextureHandle& texture, uint8_t slot) = 0;
		virtual void SetTargets(uint8_t num, TextureHandle* colorTargets, TextureHandle* depth) = 0;
		virtual void ClearTargets(uint8_t num, TextureHandle* colorTargets,float clear[4], TextureHandle* depth,float d,uint16_t stencil) = 0;
	};
}