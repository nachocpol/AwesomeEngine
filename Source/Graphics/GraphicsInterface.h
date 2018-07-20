#pragma once

#include <stdint.h>
#include <iostream>

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
		IndexBuffer = 1
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
		RGB_32_Float = 1,
		RGBA_32_Float = 2
	};
	enum Topology
	{
		InvalidTopology = 0,
		TriangleList = 1
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
		virtual GraphicsPipeline CreateGraphicsPipeline(const GraphicsPipelineDescription& desc) = 0;
		virtual ComputePipeline CreateComputePipeline(const ComputePipelineDescription& desc) = 0;
		virtual void SetBufferData(const BufferHandle& buffer, int size, int offset, void* data) = 0;
		virtual void SetVertexBuffer(const BufferHandle& buffer, int size, int eleSize) = 0;
		virtual void SetTopology(const Topology& topology) = 0;
		virtual void SetGraphicsPipeline(const GraphicsPipeline& pipeline) = 0;
		virtual void Draw(uint32_t numvtx,uint32_t vtxOffset) = 0;
		virtual void SetViewport(float x, float y, float w, float h, float zmin = 0.0f, float zmax = 1.0f) = 0;
		virtual void SetScissor(float x, float y, float w, float h) = 0;
	};
}