#pragma once

#include "CubeRenderer.h"

#include <vector>

class Room
{
public:
	// joins cubes 
	std::vector<Cube> getCubesForSave();

	std::string name = "unnamed room";

	glm::ivec3 size = { 10, 10, 10 };
	std::vector<Cube> cubes;

};

