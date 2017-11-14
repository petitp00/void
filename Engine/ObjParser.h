#pragma once

#include "Globals.h"

class ObjParser
{
public:
	ObjParser()=default;
	ObjParser(std::string filename, std::string root_folder = RESOURCES_FOLDER + "Models/");
	void ParseFile(std::string filename, std::string root_folder = RESOURCES_FOLDER + "Models/");

	std::vector<glm::vec3> data;
	std::vector<glm::vec3> vertices_data;
};
