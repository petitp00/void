#include "stdafx.h"
#include "PointRenderer.h"

Shader PointRenderer::shader;
uint PointRenderer::vao;

void PointRenderer::Init()
{
	glPointSize(5.f);

	shader.Init("primitive");

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	uint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	float data[] = { 0.f, 0.f, 0.f };
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void PointRenderer::Draw(std::vector<Point> points, PointRenderInfo info)
{
	glBindVertexArray(vao);
	shader.Use();

	shader.setMat4("projection", info.projection);
	shader.setMat4("view", info.view);

	TextToWorldRenderInfo ttwri;
	ttwri.projection = info.projection;
	ttwri.view = info.view;
	ttwri.view_pos = info.view_pos;

	for (auto p : points) {
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, p.position);
		shader.setMat4("model", model);
		shader.setVec4("color", p.color);
		glDrawArrays(GL_POINTS, 0, 3);

		if (p.display_position_text && info.text_renderer != nullptr) {
			std::stringstream ss;
			ss << p.position;
			info.text_renderer->RenderTextToWorld(ss.str(), p.position, p.text_scale, p.color, ttwri);
			glBindVertexArray(vao);
			shader.Use();
		}
	}
}
