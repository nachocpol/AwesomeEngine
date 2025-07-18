#include "Core/App/AppBase.h"
#include "Core/EntryPoint.h"
#include "Core/FileSystem.h"
#include "Core/Logging.h"

#include "Graphics/UI/UIInterface.h"
#include "Graphics/VertexDescription.h"

#include <iostream>
#include <fstream>

class ModelViewer : public AppBase
{
public:
	void Init();
	void Update()override;
	void Release() override;
};

ModelViewer app;
ENTRY_POINT(app, "Model Viewer", false);

char* OBJGetDataStart(char* data, int maxLen)
{
	for (int i = 0; i < maxLen; ++i)
	{
		if (*data == ' ')
		{
			return ++data;
		}
		++data;
	}
	return nullptr;
}

// Parses a single data line from an obj file. Line is either: v, vt or vn
// Returns the number of digits parsed or 0 if something bad happened.
int OBJParseDataLine(glm::vec4& parsedData, char* inputData, int inputLen)
{
	if (!inputData || inputLen == 0)
	{
		return 0;
	}

	char* curC = OBJGetDataStart(inputData, inputLen);

	int parsedDigits = 0;
	bool newLine = false;
	while (!newLine)
	{
		char curDigit[128];
		memset(curDigit, 0, 128);
		int counter = 0;
		bool nextDigit = false;
		while (!nextDigit)
		{
			curDigit[counter++] = *curC;
			++curC;
			if (*curC == 0x20) // Space
			{
				nextDigit = true;
				++curC; // Advance extra position							
			}
			else if (*curC == 0x0) // New line
			{
				newLine = true;
				nextDigit = true; // Stop parsing
			}
		}
		curDigit[counter] = 0;
		parsedData[parsedDigits++] = std::stof(curDigit);
	}

	return parsedDigits;
}

void ModelViewer::Init()
{
	AppBase::Init();

	FILE* file = nullptr;

	std::string objPath = "data:Models/cube.obj";
	if (Core::FileSystem::GetInstance()->FixupPath(objPath))
	{
		struct OBJData
		{
			std::vector<glm::vec3> positions;
			std::vector<glm::vec2> texCoords;
			std::vector<glm::vec3> normals;

		} dataBuffers;

		dataBuffers.positions.resize(100);
		dataBuffers.texCoords.resize(100);
		dataBuffers.normals.resize(100);

		int posCount = 0;
		int texCoordCount = 0;
		int normalsCount = 0;

		std::ifstream fileStream(objPath);
		if (fileStream.is_open())
		{
			char dataLine[128];
			while (fileStream.getline(dataLine, 128))
			{
				char schema = dataLine[0];
				char schemaB = dataLine[1];

				// Note for 'v' we ignore the fact that we could get a w value! xyz=xyz/w (implicit w = 1)
				if (schema == 'v')
				{
					if (schemaB == ' ')
					{
						glm::vec4 vertex;
						int elements = OBJParseDataLine(vertex, dataLine, 128);
						if (elements != 3)
						{
							assert(false);
						}
						dataBuffers.positions.push_back(glm::vec3(vertex));
						++posCount;
					}
					else if (schemaB == 't')
					{
						glm::vec4 texCoord;
						int elements = OBJParseDataLine(texCoord, dataLine, 128);
						if (elements != 2)
						{
							assert(false);
						}
						dataBuffers.texCoords.push_back(glm::vec2(texCoord));
						++texCoord;
					}
					else if (schemaB == 'n')
					{
						glm::vec4 normal;
						int elements = OBJParseDataLine(normal, dataLine, 128);
						if (elements != 3)
						{
							assert(false);
						}
						normal = glm::normalize(normal);
						dataBuffers.normals.push_back(glm::vec3(normal));
						++normalsCount;
					}
					else
					{
						assert(false);
					}					
				}
				else if (schema == 'f')
				{

				}
			}
		}
		INFO("Finished loading obj");
	}
	else
	{
		ERR("File: %s can not be resolved!", objPath.c_str());
	}

	// A bit nasty, need to maybe get rid of it.. On graphics creation, we leave the cmdlist open and recording, as we may submit copy commands
	// to init buffers, textures etc..
	// Probably should change the scheme and defer that copy to when we are actually in the "render frame"... Anyways :) 
	m_GraphicsInterface->FlushAndWait();
}

void ModelViewer::Update()
{
	if (ImGui::Begin("Model Viewer"))
	{
	}
	ImGui::End();
}

void ModelViewer::Release()
{
}
