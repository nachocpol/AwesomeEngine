#include "Core/App/AppBase.h"
#include "Core/EntryPoint.h"

#include "Graphics/UI/UIInterface.h"

class ModelViewer : public AppBase
{
public:
	void Init();
	void Update()override;
	void Release() override;
};

ModelViewer app;
ENTRY_POINT(app, "Model Viewer", false);

void ModelViewer::Init()
{
	AppBase::Init();
}

void ModelViewer::Update()
{
	if (ImGui::Begin("Model Viewer"))
	{
		ImGui::End();
	}
}

void ModelViewer::Release()
{
}
