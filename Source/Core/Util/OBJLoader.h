#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Core
{
	namespace OBJ
	{
		struct Triangle
		{
			int m_Positions[3];
			int m_TexCoords[3];
			int m_Normals[3];
		};

		struct Data
		{
			std::vector<glm::vec3> m_Positions;
			std::vector<glm::vec2> m_TexCoords;
			std::vector<glm::vec3> m_Normals;
			std::vector<Triangle> m_Triangles;
		};

		bool LoadFromFile(const std::string& path, Data& model);
	}
}