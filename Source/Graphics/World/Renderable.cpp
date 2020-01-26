#include "Renderable.h"
#include "Model.h"

using namespace World;

Renderable::Renderable():
	mModel(nullptr)
{
}

Renderable::~Renderable()
{
}

Graphics::Model* Renderable::GetModel() const
{
	return mModel;
}

void Renderable::SetModel(Graphics::Model* model)
{
	mModel = model;
}