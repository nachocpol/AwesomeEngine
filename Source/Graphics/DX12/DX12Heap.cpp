#include "DX12Heap.h"

namespace Graphics {namespace DX12 {

	DX12Heap::DX12Heap():
		mDeviceRef(nullptr),
		mHeap(nullptr),
		mEntrySize(0)
	{
	}

	DX12Heap::~DX12Heap()
	{
	}

	bool DX12Heap::Initialize(ID3D12Device* device, uint32_t entries, bool shaderVisible, D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		mDeviceRef = device;
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NodeMask		= 0;
		desc.NumDescriptors = entries;
		desc.Type			= type;
		desc.Flags			= shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		mDeviceRef->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap));
		mCPUHandle = mHeap->GetCPUDescriptorHandleForHeapStart();
		mGPUHandle = mHeap->GetGPUDescriptorHandleForHeapStart();
		mEntrySize = mDeviceRef->GetDescriptorHandleIncrementSize(type);
		return false;
	}

	void DX12Heap::Reset()
	{
		mCPUHandle = mHeap->GetCPUDescriptorHandleForHeapStart();
		mGPUHandle = mHeap->GetGPUDescriptorHandleForHeapStart();
	}

	void DX12Heap::OffsetHandles(uint32_t num)
	{
		mCPUHandle.Offset(num, mEntrySize);
		mGPUHandle.Offset(num, mEntrySize);
	}
}}