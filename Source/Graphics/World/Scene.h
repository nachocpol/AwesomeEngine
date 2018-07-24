#pragma once

#include "Graphics/GraphicsInterface.h"
#include <vector>
#include "glm/glm.hpp"

namespace Graphics
{
	struct Actor
	{
		Mesh AMesh;
		glm::vec3 Position;
		glm::vec3 Scale;
		glm::vec3 Rotation;
	};

	class Scene
	{
	public:
		Scene(GraphicsInterface* graphics);
		~Scene();
		bool Initialize();
		void Update(float dt);
		void Draw(float dt);
		Actor* AddActor();

	private:
		GraphicsInterface* mGraphics;
		std::vector<Actor*> mActors;
	};
}