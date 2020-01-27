#pragma once

#include "Actor.h"

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

	private:
		Graphics::Model* mModel;
	};
}