#pragma  once

#include "DX12Common.h"
#include <queue>

namespace Graphics{ namespace DX12{

	class DX12GraphicsInterface;
	class DX12ReleaseManager
	{
	public:
		DX12ReleaseManager(DX12GraphicsInterface* graphics);
		~DX12ReleaseManager();
		void Update();
		void ReleaseItem(IUnknown* item);

	private:
		struct ReleaseEntry
		{
			uint8_t FenceValue;
			ID3D12Fence* Fence;
			IUnknown* Resource;
		};
		std::queue<ReleaseEntry> mReleaseQueue;
		DX12GraphicsInterface* mGraphicsInterface;
	};

}}