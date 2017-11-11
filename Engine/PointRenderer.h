#pragma once

#include "Globals.h"
#include "Shader.h"
#include "TextRenderer.h"

struct Point
{
	glm::vec3 position = glm::vec3(0.f);
	glm::vec4 color = glm::vec4(0.f, 1.f, 0.f, 1.0f);
	bool display_position_text = false;
	float text_scale = 2.0f;
};

struct PointRenderInfo
{
	glm::mat4 projection;
	glm::mat4 view;
	TextRenderer* text_renderer = nullptr;
	glm::vec3 view_pos;
};

class PointRenderer
{
public:
	PointRenderer() = delete;
	static void Init();
	static void Draw(std::vector<Point> points, PointRenderInfo info);
private:
	static uint vao;
	static Shader shader;
};

