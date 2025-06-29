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

void ModelViewer::Init()
{
	AppBase::Init();

	FILE* file = nullptr;

	std::string objPath = "data:Models/cube.obj";
	if (Core::FileSystem::GetInstance()->FixupPath(objPath))
	{
		std::ifstream fileStream(objPath);
		if (fileStream.is_open())
		{
			char dataLine[128];
			while (fileStream.getline(dataLine, 128))
			{
				char schema = dataLine[0];
				if (schema == 'v')
				{
					float dataVector[4]; // Max of 4 elements: xyz[w] (w is optional)
					char* curC = &dataLine[2]; // Start of the data str
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
							if (*curC == 0x20) // space
							{
								nextDigit = true;
								++curC; // advance extra position							
							}
							else if (*curC == 0x0) // new line
							{
								newLine = true;
								nextDigit = true; // to stop parsing
							}
						}
						curDigit[counter] = 0;
						dataVector[parsedDigits++] = std::stof(curDigit);
					}
					INFO("Data: %f, %f, %f", dataVector[0], dataVector[1], dataVector[2]);
				}
				else if (schema == 'vt')
				{

				}
				else if (schema == 'vn')
				{

				}
				else if (schema == 'f')
				{

				}
			}
		}		
	}
	else
	{
		ERR("File: %s can not be resolved!", objPath.c_str());
	}
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
