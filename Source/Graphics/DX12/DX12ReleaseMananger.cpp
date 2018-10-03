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
			if (cur->Fence->GetCompletedValue() > 0)
			{
				cur->Resource->Release();
				cur->Fence->Release();
				mReleaseQueue.pop();
			}
			else
			{
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
			mReleaseQueue.push({ fence, item });
		}
	}

}}