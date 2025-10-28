#pragma once

namespace  Core
{
	enum class KeyState
	{
		Released = 0,
		Pressed,
	};

	enum class KeyType
	{
		Num1 = 0,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,
		Num0,
		
		A, B, C, D, E, F, G, H, I, J, K, L, M, 
		N, O, P ,Q , R, S, T, U, V, W, X, Y, Z,

		Escape,
		Space,
		Tab,
		Return,

		COUNT
	};

	enum class MouseButton
	{
		Left = 0,
		Right,
		Middle,
		COUNT
	};
}