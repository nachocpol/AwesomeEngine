#include "DebugDraw.h"

using namespace Graphics;

DebugDraw::DebugDraw()
{
}

DebugDraw::~DebugDraw()
{
}

DebugDraw* Graphics::DebugDraw::GetInstance()
{
	return nullptr;
}

void DebugDraw::Initialize(GraphicsInterface* graphicsInterface)
{
}

void DebugDraw::Release()
{
}

void DebugDraw::DrawLine(glm::vec3 start, glm::vec3 end, glm::vec4 color)
{
}
