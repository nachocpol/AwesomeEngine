#pragma once

#include "Model.h"
namespace Graphics
{
	class GraphicsInterface;
}

namespace World
{
	class CameraComponent;
	class LandscapeComponent : public ModelComponent
	{
	public:
		LandscapeComponent();

		void SetSourceHeightMap(std::string heightMap);

	private:
		void Render(Graphics::GraphicsInterface* graphicsInterface, World::CameraComponent* camera);
		
		std::string mSourceHeightMap;
	};
}