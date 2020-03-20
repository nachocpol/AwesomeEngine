#include "ProbeComponent.h"

using namespace World;
using namespace Graphics;

ProbeComponent::ProbeComponent()
	: mSourceEnvMap("assets:Textures/Envmaps/sunflowers_1k.hdr")
	, mShape(ProbeShape::Infinite)
	, mInitialized(false)
	, SourceTexture(InvalidTexture)
	, IrradianceTexture(InvalidTexture)
	, ConvolutedTexture(InvalidTexture)
{
}

ProbeComponent::~ProbeComponent()
{
}

void ProbeComponent::SetSourceEnvMap(const char* path)
{
	std::string newPath = path;
	if (newPath != path)
	{
		mInitialized = false;
		mSourceEnvMap = newPath;
	}
}

void ProbeComponent::Update(float deltaTime)
{
}

const char* World::ProbeComponent::GetSourcePath() const
{
	return mSourceEnvMap.c_str();
}
