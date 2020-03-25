#pragma once	

namespace World
{
	class Actor;
	class Component
	{
		friend Actor;
	public:
		virtual ~Component() {}

		virtual void UpdatePhysics() {};
		virtual void Update(float deltaTime) = 0;
		virtual void UpdateLate() {};
		virtual void RenderUI() {};

		Actor* GetParent()const { return mParent; }

	protected:
		Actor* mParent = nullptr;
	};
}