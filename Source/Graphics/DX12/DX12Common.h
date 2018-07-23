#pragma once

#include "d3dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#ifdef _WIN32
	#pragma comment(lib,"d3d12")
	#pragma comment(lib,"dxgi")
	#pragma comment(lib,"d3dcompiler")

	#define NUM_BACK_BUFFERS		3
	#define MAX_BUFFERS				256
	#define MAX_TEXTURES			256
	#define MAX_GRAPHICS_PIPELINES  256

	#define VERTEX_CB_READ			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
	#define INDEX_READ				D3D12_RESOURCE_STATE_INDEX_BUFFER
	#define COPY_DST				D3D12_RESOURCE_STATE_COPY_DEST
	#define COPY_SRC				D3D12_RESOURCE_STATE_COPY_SOURCE
	#define TEXTURE_READ			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE 

#else
	#error This platform is not supported
#endif