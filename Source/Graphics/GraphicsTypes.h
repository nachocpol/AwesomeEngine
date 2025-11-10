#pragma once

#include <stdint.h>

namespace Graphics
{
	enum class ShaderType
	{
		Invalid = 0,
		Vertex = 1,
		Pixel = 2,
		Compute = 3
	};

	enum class BufferType
	{
		VertexBuffer,
		IndexBuffer,
		ConstantBuffer,
		GPUBuffer
	};

	enum class GPUQueryType
	{
		Timestamp
	};

	struct CPUAccess
	{
		enum T
		{
			Read,
			Write,
			ReadWrite,
			None
		};
	};

	struct GPUAccess
	{
		enum T
		{
			Read,
			ReadWrite,
			None
		};
	};

	enum class Format
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
		R_11_G_11_B_10_Float = 12,
		Depth32_Float = 13
	};

	struct Primitive
	{
		enum T
		{
			Undefined,
			Point,
			Line,
			Triangle,
			Patch
		};
	};

	enum class Topology
	{
		InvalidTopology,
		TriangleList,
		LineList,
		LineStrip,
	};

	struct FaceCullMode
	{
		enum T
		{
			None,
			Back,
			Front
		};
	};

	struct TextureFlags
	{
		enum T
		{
			TextureFlagNone = 0,
			RenderTarget = 1 << 1,
			DepthStencil = 1 << 2,
			UnorderedAccess = 1 << 3
		};
	};

	struct DepthFunc
	{
		enum T
		{
			Always = 0,
			Never = 1,
			Equal = 2,
			LessEqual = 3,
			GreatEqual = 4
		};
	};

	struct BlendOperation
	{
		enum T
		{
			BlendOpAdd = 0,
			BlendOpSubstract = 1,
			BlendOpMin = 2,
			BlendOpMax = 3
		};
	};

	struct BlendFunction
	{
		enum T
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

	static const BufferHandle InvalidBuffer = { UINT64_MAX };
	static const TextureHandle InvalidTexture = { UINT64_MAX };
	static const GraphicsPipeline InvalidGraphicsPipeline = { UINT64_MAX };
	static const ComputePipeline InvalidComputePipeline = { UINT64_MAX };

	static bool IsDepthFormat(Format f)
	{
		if (f == Format::Depth24_Stencil8 || f == Format::Depth32_Float)
		{
			return true;
		}
		return false;
	}
}