#include "LandscapeComponent.h"
#include "Graphics/GraphicsInterface.h"

using namespace World;
using namespace  Graphics;

LandscapeComponent::LandscapeComponent()
{
	//RenderCallback cb = &LandscapeComponent::Render;

	//SetRenderCallback(&LandscapeComponent::Render);
}

void LandscapeComponent::SetSourceHeightMap(std::string heightMap)
{
}

void LandscapeComponent::Render(Graphics::GraphicsInterface* graphicsInterface, World::CameraComponent* camera)
{
}
