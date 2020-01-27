#include "InputManager.h"
#include <memory>
#include <Windows.h>

using namespace Core;

InputManager::InputManager()
{
	memset(KeyStates, 0, sizeof(KeyStates));
	memset(SpecialKeyStates, 0, sizeof(SpecialKeyStates));
	memset(MouseButtonStates, 0, sizeof(MouseButtonStates));
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

bool InputManager::IsKeyPressed(char key)
{
	// TO-DO: fix win input
	if (KeyStates[key - 32] == StateDown)
	{
		return true;
	}
	return false;
}

bool InputManager::IsSpecialKeyPressed(SpecialKey key)
{
	if (SpecialKeyStates[key] == StateDown)
	{
		return true;
	}
	return false;
}

bool InputManager::IsMouseButtonPressed(MouseButton btn)
{
	if (MouseButtonStates[btn] == StateDown)
	{
		return true;
	}
	return false;
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
