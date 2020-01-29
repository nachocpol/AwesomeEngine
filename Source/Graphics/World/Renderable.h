#pragma once

#include "Actor.h"
#include "Graphics/GraphicsInterface.h"

#include "glm/glm.hpp"

namespace Graphics
{
	struct Model;
}

namespace World
{
	class Renderable : public Actor
	{
	public:
		Renderable();
		~Renderable();

		Graphics::Model* GetModel()const;
		void SetModel(Graphics::Model* model);

		Type::T GetActorType() const { return Type::Renderable; }

		Graphics::AABBData GetWorldAABB(uint32_t meshIdx);

	private:
		Graphics::Model* mModel;
	};
}