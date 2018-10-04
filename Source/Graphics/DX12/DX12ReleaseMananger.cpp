#include "DX12ReleaseMananger.h"
#include "DX12GraphicsInterface.h"

namespace Graphics{namespace DX12{

	DX12ReleaseManager::DX12ReleaseManager(DX12GraphicsInterface* graphics):
		mGraphicsInterface(graphics)
	{
	}

	DX12ReleaseManager::~DX12ReleaseManager()
	{
	}

	void DX12ReleaseManager::Update()
	{
		bool keepChecking = true;
		while (mReleaseQueue.size() && keepChecking)
		{
			ReleaseEntry* cur = &mReleaseQueue.front();
			// We need to wait for 3 frames at least!
			if (cur->Fence->GetCompletedValue() == 3)
			{
				cur->Resource->Release();
				cur->Fence->Release();
				mReleaseQueue.pop();
			}
			else
			{
				mGraphicsInterface->mDefaultSurface.Queue->Signal(cur->Fence, cur->FenceValue++);
				keepChecking = false;
			}
		}
	}

	void DX12ReleaseManager::ReleaseItem(IUnknown* item)
	{
		if (item)
		{
			// Set a fence event:
			ID3D12Fence* fence = nullptr;
			mGraphicsInterface->mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
			mGraphicsInterface->mDefaultSurface.Queue->Signal(fence, 1);

			ReleaseEntry entry;
			entry.FenceValue = 1;
			entry.Fence = fence;
			entry.Resource = item;
			mReleaseQueue.push(entry);
		}
	}

}}