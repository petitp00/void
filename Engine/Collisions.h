#pragma once

#include "CubeMesh.h"

namespace Collision
{
	struct Result {
		bool collision;
		glm::vec3 mtv; // minimum translation vector
	};

	struct Box {
		Box() = default;
		Box(Cube cube);
		glm::vec3 position;
		glm::vec3 size;
	};

	struct HalfBox {
		HalfBox() = default;
		HalfBox(HalfCube hcube);
		glm::vec3 position;
		glm::vec3 size;
		glm::vec2 rotation;
		glm::mat4 rotation_matrix;
	};

	Result Get(Box b1, Box b2);
	Result Get(Box b1, HalfBox b2, glm::vec3& start1, glm::vec3& end1, glm::vec3& start2, glm::vec3& end2);
}