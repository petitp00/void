#pragma once

#include "../Engine/LineRenderer.h"

class Grid
{
public:
	Grid(int size = 50, float scale=1.0f, glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), float thickness = 0.1f);
	void Draw(LineRenderInfo info);

private:
	uint vao;
	Shader shader;
	uint size;
	glm::vec4 color;
};

