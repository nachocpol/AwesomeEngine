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

	enum SpecialKey
	{
		F1 = 0,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		TAB,
		ESC,
		SpecialKeyCount
	};

	enum MouseButton
	{
		Left = 0,
		Right,
		Middle,
		MouseButtonCount
	};

	class InputManager
	{
	public:
		static InputManager* GetInstance();
		bool IsKeyPressed(char key);
		bool IsSpecialKeyPressed(SpecialKey key);
		bool IsMouseButtonPressed(MouseButton btn);
		glm::vec2 GetMousePos();
		KeyState KeyStates[256];
		KeyState SpecialKeyStates[SpecialKeyCount];
		KeyState MouseButtonStates[MouseButtonCount];
		// Platform window handle
		void* WHandle;

	private:
		InputManager();
		~InputManager();
	};
}}