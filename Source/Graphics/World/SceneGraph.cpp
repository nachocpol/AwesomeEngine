#include "SceneGraph.h"
#include "Actor.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "TransformComponent.h"
#include "PhysicsWorld.h"

#include "Graphics/UI/IMGUI/imgui.h"

using namespace World;

SceneGraph::SceneGraph()
	:mSelectedActor(nullptr)
{
}

SceneGraph::~SceneGraph()
{
}

void World::SceneGraph::Initialize()
{
	// Physics representation of this scene graph:
	PhysicsWorld::GetInstance()->Initialize();
	
	// Add root entity:
	mRoot = new Actor;
	mRoot->AddComponent<TransformComponent>();
	mRoot->mSceneOwner = this;
}

void SceneGraph::UpdatePhysics(float deltaTime)
{
	// Update physics world:
	PhysicsWorld::GetInstance()->Update(deltaTime);

	// Then bring both in sync:
	mRoot->UpdatePhysics();
}

void SceneGraph::Update(float deltaTime)
{
	// Generic component update, figure world transformations etc.
	mRoot->Update(deltaTime);

	// SceneGraph -> PhysicsWorld sync back. Also update things that need position/transforms to be ready.
	mRoot->UpdateLate();
}

void SceneGraph::RenderUI()
{
	static bool kShowGraph = true;
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("Scene Graph"))
	{
		ImGui::Checkbox("Show Graph", &kShowGraph);
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();

	if (kShowGraph)
	{
		int totalh = 1080;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
		ImGui::SetNextWindowPos(ImVec2(0, 16));
		ImGui::SetNextWindowSize(ImVec2(256, 512));
		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::Begin("Graph", &kShowGraph, flags);
		{
			ImGui::Text("Scene Graph");
			ImGui::Separator();
			RenderGraphTree(mRoot->GetChilds());
		}
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(0, 512 + 16));
		ImGui::SetNextWindowSize(ImVec2(256, totalh - (512 + 16)));
		ImGui::SetNextWindowBgAlpha(0.5f);
		ImGui::Begin("Properties", &kShowGraph, flags);
		{
			ImGui::Text("Properties");
			ImGui::Separator();
		}
		ImGui::End();

		ImGui::PopStyleVar(2);
	}
}

Actor* SceneGraph::SpawnActor(Actor* parent)
{
	Actor* actor = new Actor;
	actor->mSceneOwner = this;
	if (parent)
	{
		parent->AddChild(actor);
	}
	else
	{
		mRoot->AddChild(actor);
	}
	return actor;
}

Actor* World::SceneGraph::GetRoot() const
{
	return mRoot;
}

void SceneGraph::RenderGraphTree(const std::vector<Actor*>& actors)
{
	for (const Actor* actor : actors)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (actor->GetNumChilds() == 0)
		{
			// Current is a leaf!
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}
		if (mSelectedActor == actor)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}
		if (ImGui::TreeNodeEx(actor,flags,"Actor"))
		{
			if (ImGui::IsItemClicked())
			{
				mSelectedActor = (Actor*)actor;
			}
			RenderGraphTree(actor->GetChilds());
			if (!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
			{
				ImGui::TreePop();
			}
		}
		else
		{
			if (ImGui::IsItemClicked())
			{
				mSelectedActor = (Actor*)actor;
			}
		}
	}
}
