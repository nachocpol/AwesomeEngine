#pragma once

#include "Component.h"

#include <vector>

namespace World
{
	class SceneGraph;
	class TransformComponent;
	class RigidBodyComponent;
	class Actor
	{
		friend SceneGraph;
	public:
		Actor();
		~Actor();

		template<class T>
		T* AddComponent();

		template<class T>
		T* FindComponent(bool recursive = false);

		template<class T>
		void FindComponents(std::vector<T*>& components, bool recursive = false);

		uint32_t GetNumChilds()const;
		Actor* GetChild(uint32_t index);
		const std::vector<Actor*>& GetChilds()const;

		Actor* GetParent()const;

		virtual void UpdatePhysics();
		virtual void Update(float deltaTime);
		virtual void UpdateLate();

		// Simple getter for the transform component:
		TransformComponent* Transform = nullptr;

	protected:
		void AddChild(Actor* child);

		Actor* mParent;
		std::vector<Actor*> mChilds;
		std::vector<Component*> mComponents;
		SceneGraph* mSceneOwner;
	};

	template<>
	TransformComponent* Actor::AddComponent();

	template<>
	RigidBodyComponent* Actor::AddComponent();

	template<class T>
	inline T* Actor::AddComponent()
	{
		T* component = new T();
		mComponents.push_back(component);
		component->mParent = this;
		return component;
	}

	template<class T>
	inline T* Actor::FindComponent(bool recursive)
	{
		for (Component* c : mComponents)
		{
			T* casted = dynamic_cast<T*>(c);
			if (casted)
			{
				return casted;
			}
		}
		if (recursive)
		{
			for (Actor* child : mChilds)
			{
				T* childComp = child->FindComponent<T>(recursive);
				if (childComp)
				{
					return childComp;
				}
			}
		}
		return nullptr;
	}

	template<class T>
	inline void Actor::FindComponents(std::vector<T*>& components, bool recursive)
	{
		for (Component* c : mComponents)
		{
			T* casted = dynamic_cast<T*>(c);
			if (casted)
			{
				components.push_back(casted);
			}
		}
		if (recursive)
		{
			for (Actor* child : mChilds)
			{
				child->FindComponents<T>(components, recursive);
			}
		}
	}
}