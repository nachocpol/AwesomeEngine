#include "Core/App/AppBase.h"
#include "Core/EntryPoint.h"
#include "Core/FileSystem.h"
#include "Core/Logging.h"

#include "Graphics/Platform/Windows/WWindow.h"
#include "Graphics/UI/UIInterface.h"
#include "Graphics/VertexDescription.h"

#include "Samples/ModelViewer.hlsl" // This is a bit nasty... we are forced to include here the file so we can see the declarations.

#include "glm/glm.hpp"

#include <iostream>
#include <fstream>

class ModelViewer : public AppBase
{
public:
	void Init();
	void Update()override;
	void Release() override;
private:
	Graphics::BufferHandle m_PositionsBuffer;
	Graphics::GraphicsPipeline m_PSO;
	Graphics::BufferHandle m_CB;
	Declarations::ViewerConstants m_Constants;
	int m_NumVertices;
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

	struct OBJTriangle
	{
		int positions[3];
		int texCoords[3];
		int normals[3];
	};

	struct OBJData
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> texCoords;
		std::vector<glm::vec3> normals;

		std::vector<OBJTriangle> triangles;

	} dataBuffers;

	std::string objPath = "data:Models/suzanne.obj";
	if (Core::FileSystem::GetInstance()->FixupPath(objPath))
	{	
		dataBuffers.positions.reserve(100);
		dataBuffers.texCoords.reserve(100);
		dataBuffers.normals.reserve(100);

		int posCount = 0;
		int texCoordCount = 0;
		int normalsCount = 0;

		std::ifstream fileStream(objPath);
		if (fileStream.is_open())
		{
			char dataLine[128];
			while (fileStream.getline(dataLine, 128))
			{
				const int lineLen = (int)strlen(dataLine);
				char schema = dataLine[0];
				char schemaB = dataLine[1];

				// Parsing like this, asumes all data will come sequentially (no mixed f,v etc..) not sure if obj even supports it?

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
					// Figure out if its a triangle
					{
						char* fCur = dataLine;
						int spaces = 0;
						while (*fCur != '\0')
						{
							if (*fCur == ' ')
							{
								++spaces;
							}
							++fCur;
						}

						// we should have 3 spaces for a triangle:  f 1/1/1 3/3/3 5/5/5
						if (spaces != 3)
						{
							ERR("Unsupported OBJ");
							assert(false);
						}
					}	

					// Now parse it
					bool hasNormals = dataBuffers.normals.size();
					bool hasTexCoord = dataBuffers.positions.size();

					if (!hasNormals || !hasTexCoord)
					{
						assert(false);
					}

					// Insert a space so we let the parsing code handle the last index (otherwise it will stop before processing it..)
					dataLine[lineLen] = ' ';
					dataLine[lineLen + 1] = '\0';

					// Prepare to parse
					char* curChar = dataLine;
					++curChar; //f
					++curChar;// space

					OBJTriangle curTriangle;

					int curTriIndex = 0;
					int curEleIndex = 0;
					char curDigit[128];
					char* digitPtr = curDigit;
					while (*curChar != '\0' )
					{
						if (*curChar != '/' && *curChar != ' ')
						{
							*digitPtr = *curChar;
							++digitPtr;							
						}						
						else
						{
							const int curIndex = std::stoi(curDigit) - 1; // OBJ arrays start at 1

							if (curEleIndex == 0)
							{
								curTriangle.positions[curTriIndex] = curIndex;
							}
							else if (curEleIndex == 1)
							{
								curTriangle.texCoords[curTriIndex] = curIndex;
							}
							else if (curEleIndex == 2)
							{
								curTriangle.normals[curTriIndex] = curIndex;

								// Done with this vertex.
								++curTriIndex;
								curEleIndex = 0;
							}

							// Reset digit
							digitPtr = curDigit;
							memset(curDigit, 0, 128);

							if (*curChar != ' ')
							{
								++curEleIndex;
							}
						}

						++curChar;
					}

					dataBuffers.triangles.push_back(curTriangle);
				}
			}
		}
		INFO("Finished loading obj");
	}
	else
	{
		ERR("File: %s can not be resolved!", objPath.c_str());
	}

	std::vector<Graphics::PosVertexDescription> positionsBuffer;

	for (int i = 0; i < dataBuffers.triangles.size(); ++i)
	{
		const OBJTriangle& curTri = dataBuffers.triangles[i];
		for (int j = 0; j < 3; ++j)
		{
			const glm::vec3 curPos = dataBuffers.positions[curTri.positions[j]];
			Graphics::PosVertexDescription cur;
			cur.m_Position[0] = curPos.x;
			cur.m_Position[1] = curPos.y;
			cur.m_Position[2] = curPos.z;
			positionsBuffer.push_back(cur);
		}
	}
	 
	m_NumVertices = (int)positionsBuffer.size();
	m_PositionsBuffer = m_GraphicsInterface->CreateBuffer(
		Graphics::BufferType::GPUBuffer, Graphics::CPUAccess::None, Graphics::GPUAccess::Read, sizeof(Graphics::PosVertexDescription), m_NumVertices, positionsBuffer.data()
	);

	Graphics::GraphicsPipelineDescription psoDesc;
	psoDesc.ColorFormats[0] = m_GraphicsInterface->GetOutputFormat();
	
	psoDesc.VertexShader.ShaderPath = "shadersrc:Samples/ModelViewer.hlsl";
	psoDesc.VertexShader.ShaderEntryPoint = "VSMain";
	psoDesc.VertexShader.Type = Graphics::ShaderType::Vertex;
	
	psoDesc.PixelShader.ShaderPath = "shadersrc:Samples/ModelViewer.hlsl";
	psoDesc.PixelShader.ShaderEntryPoint = "PSMain";
	psoDesc.PixelShader.Type = Graphics::ShaderType::Pixel;

	m_PSO = m_GraphicsInterface->CreateGraphicsPipeline(psoDesc);

	m_CB = m_GraphicsInterface->CreateBuffer(Graphics::BufferType::ConstantBuffer, Graphics::CPUAccess::None, Graphics::GPUAccess::Read, sizeof(m_Constants));

	// A bit nasty, need to maybe get rid of it.. On graphics creation, we leave the cmdlist open and recording, as we may submit copy commands
	// to init buffers, textures etc..
	// Probably should change the scheme and defer that copy to when we are actually in the "render frame"... Anyways :) 
	m_GraphicsInterface->FlushAndWait();
}

void ModelViewer::Update()
{
	glm::mat4x4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4x4 proj = glm::perspectiveFov(90.0f, (float)m_Window->GetWidth(), (float)m_Window->GetHeight(), 0.1f, 100.0f);

	m_Constants.Model = glm::rotate(glm::mat4x4(), TotalTime, glm::vec3(1.0f, 0.0f, 0.0f));
	m_Constants.Model = glm::rotate(m_Constants.Model, TotalTime, glm::vec3(0.0f, 1.0f, 0.0f));
	m_Constants.Model = glm::rotate(m_Constants.Model, -TotalTime, glm::vec3(0.0f, 0.0f, 1.0f));

	m_Constants.ViewProjection = proj * view;

	m_GraphicsInterface->SetScissor(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

	m_GraphicsInterface->SetConstantBuffer(m_CB, Declarations::kViewerConstantsSlot, sizeof(m_Constants), &m_Constants);

	m_GraphicsInterface->SetTopology(Graphics::Topology::TriangleList);
	m_GraphicsInterface->SetGraphicsPipeline(m_PSO);
	m_GraphicsInterface->SetResource(m_PositionsBuffer, 0);

	m_GraphicsInterface->Draw(m_NumVertices, 0);
}

void ModelViewer::Release()
{
}
