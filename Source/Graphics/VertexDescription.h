#pragma once

#include <stdint.h>

namespace Graphics
{
	struct VertexInputDescription;

	struct PosVertexDescription
	{
		float m_Position[3];
		static const VertexInputDescription& GetDescription();
	};

	struct PosColorVertexDescription
	{
		float m_Position[3];
		float m_Color[3];
		static const VertexInputDescription& GetDescription();
	};

	struct Pos2TexCoordColorDescription
	{
		float m_Position[2];
		float m_TexCoord[2];
		uint32_t m_Color;
		static const VertexInputDescription& GetDescription();
	};

	struct PosNormalTangentTexCoordDescription
	{
		float m_Position[3];
		float m_Normal[3];
		float m_Tangent[3];
		float m_TexCoord[2];
		static const VertexInputDescription& GetDescription();
	};
}