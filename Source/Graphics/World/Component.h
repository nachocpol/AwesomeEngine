#pragma once	

namespace World
{
	class Actor;
	class Component
	{
		friend Actor;
	public:
		virtual ~Component() {}
		virtual void Update(float deltaTime) = 0;
	protected:
		Actor* mParent = nullptr;
	};
}