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

	private:
		Graphics::Model* mModel;
	};
}