#include "OBJLoader.h"

#include "Core/FileSystem.h"
#include "Core/Logging.h"

#include <iostream>
#include <fstream>

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

bool Core::OBJ::LoadFromFile(const std::string& path, Data& model)
{
	bool loadead = false;
	std::string fixedPath(path);

	// Check if the path is already resolved
	if (!Core::FileSystem::GetInstance()->FileExists(path))
	{
		// If not, try to resolve
		if (!Core::FileSystem::GetInstance()->FixupPath(fixedPath))
		{
			ERR("File: %s can not be resolved!", fixedPath.c_str());

			return loadead;
		}
	}

	model.m_Positions.reserve(1000);
	model.m_TexCoords.reserve(1000);
	model.m_Normals.reserve(1000);

	int posCount = 0;
	int texCoordCount = 0;
	int normalsCount = 0;

	std::ifstream fileStream(fixedPath);
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
					model.m_Positions.push_back(glm::vec3(vertex));
					++posCount;
				}
				else if (schemaB == 't')
				{
					glm::vec4 texCoord;
					int elements = OBJParseDataLine(texCoord, dataLine, 128);
					if (!(elements == 2 || elements == 3))
					{
						assert(false);
					}
					model.m_TexCoords.push_back(glm::vec2(texCoord));
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
					model.m_Normals.push_back(glm::vec3(normal));
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
				bool hasNormals = model.m_Normals.size();
				bool hasTexCoord = model.m_Positions.size();

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

				Core::OBJ::Triangle curTriangle;

				int curTriIndex = 0;
				int curEleIndex = 0;
				char curDigit[128];
				char* digitPtr = curDigit;
				while (*curChar != '\0')
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
							curTriangle.m_Positions[curTriIndex] = curIndex;
						}
						else if (curEleIndex == 1)
						{
							curTriangle.m_TexCoords[curTriIndex] = curIndex;
						}
						else if (curEleIndex == 2)
						{
							curTriangle.m_Normals[curTriIndex] = curIndex;

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

				model.m_Triangles.push_back(curTriangle);
			}
		}
		loadead = true;
	}
	else
	{
		ERR("Failed to open the file stream: %s", fixedPath.c_str());
	}

	return loadead;
}
