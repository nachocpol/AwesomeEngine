#include "VertexDescription.h"
#include "GraphicsInterface.h"

using namespace Graphics;


const VertexInputDescription& PosVertexDescription::GetDescription()
{
	static VertexInputDescription::VertexInputElement k_Elements[] = {
		{"POSITION", 0, Format::RGB_32_Float, 0}
	};
	static const VertexInputDescription k_Desc = { _countof(k_Elements),k_Elements,false };
	return k_Desc;
}

const VertexInputDescription& PosColorVertexDescription::GetDescription()
{
	static VertexInputDescription::VertexInputElement k_Elements[] = {
		{"POSITION", 0, Format::RGB_32_Float, 0},
		{"COLOR", 0, Format::RGB_32_Float, 12}
	};
	static const VertexInputDescription k_Desc = { _countof(k_Elements),k_Elements,false };
	return k_Desc;
}

const VertexInputDescription& Graphics::Pos2TexCoordColorDescription::GetDescription()
{
	static VertexInputDescription::VertexInputElement k_Elements[] = {
		{"POSITION", 0, Format::RG_32_Float, 0},
		{"TEXCOORD", 0, Format::RG_32_Float, 8},
		{"COLOR", 0, Format::RGBA_8_Unorm, 16}
	};
	static const VertexInputDescription k_Desc = { _countof(k_Elements),k_Elements,false };
	return k_Desc;
}

const VertexInputDescription& PosNormalTangentTexCoordDescription::GetDescription()
{
	static VertexInputDescription::VertexInputElement k_Elements[] = {
		{"POSITION", 0, Format::RGB_32_Float, 0},
		{"NORMAL", 0, Format::RGB_32_Float, 12},
		{"TANGENT", 0, Format::RGB_32_Float, 24},
		{"TEXCOORD", 0, Format::RG_32_Float, 36},
	};
	static const VertexInputDescription k_Desc = { _countof(k_Elements),k_Elements,false };
	return k_Desc;
}
