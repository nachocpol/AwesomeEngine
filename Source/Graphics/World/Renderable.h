#pragma once

#include "Actor.h"
#include "Graphics/GraphicsInterface.h"
#include "Core/Math.h"

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

		void UpdateBounds()override;
		Math::AABBData GetWorldAABB(uint32_t meshIdx);
		Math::BSData GetWorldBS(uint32_t meshIdx);

	private:
		Graphics::Model* mModel;
		std::vector<Math::BSData> mWorldBS;
		std::vector<Math::AABBData> mWorldAABB;
	};
}