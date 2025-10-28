#pragma once

#include "InputTypes.h"

#include "glm/fwd.hpp"

#include <stdint.h>

namespace Core
{
	typedef void (*KeyEventCbk)(KeyType key, KeyState state);
	typedef void (*MouseButtonCbk)(MouseButton button, bool pressed);
	typedef void (*InputCharCbk)(unsigned short v);

	class InputManager
	{
	public:
		static InputManager* GetInstance();
		
		bool IsKeyPressed(KeyType type);
		bool IsMouseButtonPressed(MouseButton btn);
		glm::vec2 GetMousePos();

		// Notifies the InputManager that a key state has changed.
		void KeyEvent(KeyType key, KeyState state);

		void MouseButtonEvent(MouseButton button, bool pressed);

		void InputCharEvent(unsigned short v);
		
		// Configures a callback, when a key state changes cbk will be notified. 
		// Right now, only one callback is supported
		void SetKeyEventCallback(KeyEventCbk cbk);

		void SetMouseButtonCallback(MouseButtonCbk cbk);	

		void SetInputCharCallback(InputCharCbk cbk);
		
		// Platform window handle
		void* WHandle;

	private:
		KeyState m_KeyStates[(int)KeyType::COUNT];
		bool m_MouseButtonStates[(int)MouseButton::COUNT];

		KeyEventCbk m_KeyEventCbk;
		MouseButtonCbk m_MouseButtonCbk;
		InputCharCbk m_InputCharCbk;

		InputManager();
		~InputManager();
	};
}