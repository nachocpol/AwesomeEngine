#pragma once

#include "glm/glm.hpp"
#include <stdint.h>

namespace Graphics{ namespace Platform
{
	enum KeyState
	{
		StateUp = 0,
		StateDown = 1
	};

	class InputManager
	{
	public:
		static InputManager* GetInstance();
		bool IsKeyPressed(char key);
		glm::vec2 GetMousePos();
		KeyState KeyStates[256];
		// Platform window handle
		void* WHandle;

	private:
		InputManager();
		~InputManager();
	};
}}