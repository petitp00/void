#pragma once

#include "Globals.h"
#include "Shader.h"

struct Line
{
	glm::vec3 start, end;
	glm::vec4 color;
	float thickness = 1.f;
};

struct LineRenderInfo
{
	glm::mat4 projection;
	glm::mat4 view;
};

class LineRenderer
{
public:
	LineRenderer()=delete;
	static void Init();
	static void Draw(std::vector<Line> lines, LineRenderInfo info);
private:
	static uint vao;
	static Shader shader;
	static bool initialized;
};

