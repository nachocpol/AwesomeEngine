#pragma once

#include "DX12Common.h"
#include <stdint.h>

namespace Graphics{ namespace DX12{

	class DX12Heap
	{
	public:
		DX12Heap();
		~DX12Heap();
		bool Initialize(ID3D12Device* device, uint32_t entries, bool shaderVisible, D3D12_DESCRIPTOR_HEAP_TYPE type);
		void Reset();
		void OffsetHandles(uint32_t num);
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPU() { return mCPUHandle; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPU() { return mGPUHandle; }
		ID3D12DescriptorHeap* GetHeap() { return mHeap; }

	private:
		ID3D12Device* mDeviceRef;
		ID3D12DescriptorHeap* mHeap;
		CD3DX12_CPU_DESCRIPTOR_HANDLE mCPUHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE mGPUHandle;
		UINT mEntrySize;
	};
}}