#pragma once

#include "GraphicsInterface.h"

namespace Graphics
{
	class CloudRenderer
	{
	public:
		CloudRenderer();
		~CloudRenderer();
		bool Initialize(GraphicsInterface* graphicsInterface);
		void Draw(float dt);
		void ShowDebug();

	private:
		TextureHandle mTestTexture;
		GraphicsInterface* mGraphicsInterface;
	};
	
}