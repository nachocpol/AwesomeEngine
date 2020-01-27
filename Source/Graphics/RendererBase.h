#pragma once

class AppBase;

namespace World
{
	class SceneGraph;
}
namespace Graphics
{
	class GraphicsInterface;
	class RendererBase
	{
	public:
		RendererBase() {} 
		virtual ~RendererBase() {};
		virtual void Initialize(AppBase* app) = 0;
		virtual void Release() = 0;
		virtual void Render(World::SceneGraph* scene) = 0;

	protected:
		AppBase* mOwnerApp;
		GraphicsInterface* mGraphicsInterface;
	};
}