#include "InputManager.h"

#include "glm/glm.hpp"

#include "Graphics/UI/IMGUI/imgui.h"

#include <Windows.h>

using namespace Core;

InputManager::InputManager()
	: m_KeyEventCbk(nullptr)
	, m_MouseButtonCbk(nullptr)
	, m_InputCharCbk(nullptr)
{
	memset(m_KeyStates, 0, sizeof(m_KeyStates));
	memset(m_MouseButtonStates, 0, sizeof(m_MouseButtonStates));
}

InputManager::~InputManager()
{

}

InputManager* InputManager::GetInstance()
{
	static InputManager* sInstance = nullptr;
	if (!sInstance)
	{
		sInstance = new InputManager;
	}
	return sInstance;
}

bool InputManager::IsKeyPressed(KeyType type)
{
	return m_KeyStates[(int)type] == KeyState::Pressed;
}

bool InputManager::IsMouseButtonPressed(MouseButton btn)
{
	return m_MouseButtonStates[(int)btn];
}

glm::vec2 InputManager::GetMousePos()
{
	POINT p;
	if (GetCursorPos(&p))
	{
		if (ScreenToClient((HWND)WHandle, &p))
		{
			return glm::vec2(p.x, p.y);
		}
	}
	return glm::vec2(-1.0f, -1.0f);
}

void InputManager::KeyEvent(KeyType key, KeyState state)
{
	m_KeyStates[(int)key] = state;

	if (m_KeyEventCbk)
	{
		m_KeyEventCbk(key, state);
	}
}

void InputManager::MouseButtonEvent(MouseButton button, bool pressed)
{
	m_MouseButtonStates[(int)button] = pressed;

	if (m_MouseButtonCbk)
	{
		m_MouseButtonCbk(button, pressed);
	}
}

void InputManager::InputCharEvent(unsigned short v)
{
	if (m_InputCharCbk)
	{
		m_InputCharCbk(v);
	}
}

void InputManager::SetKeyEventCallback(KeyEventCbk cbk)
{
	if (m_KeyEventCbk)
	{
		assert(false);
	}

	m_KeyEventCbk = cbk;
}

void InputManager::SetMouseButtonCallback(MouseButtonCbk cbk)
{
	if (m_MouseButtonCbk)
	{
		assert(false);
	}

	m_MouseButtonCbk = cbk;
}

void InputManager::SetInputCharCallback(InputCharCbk cbk)
{
	if (m_InputCharCbk)
	{
		assert(false);
	}

	m_InputCharCbk = cbk;
}
