#pragma once

#include "Globals.h"

class ObjParser
{
public:
	ObjParser()=default;
	ObjParser(cstr filename, cstr root_folder = "../Resources/Models/");
	void ParseFile(cstr filename, cstr root_folder="../Resources/Models/");

	std::vector<glm::vec3> data;
	std::vector<glm::vec3> vertices_data;
	//std::vector<glm::vec3> vertices;
	//std::vector<glm::vec3> normals;
};
