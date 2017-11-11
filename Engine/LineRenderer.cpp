#include "stdafx.h"
#include "LineRenderer.h"
#include "ObjParser.h"

uint LineRenderer::vao;
Shader LineRenderer::shader;
bool LineRenderer::initialized = false;

void LineRenderer::Init()
{
	if (!initialized) {
		initialized = true;
		shader.Init("primitive");
		ObjParser parser("cubeline.obj");

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		uint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * parser.vertices_data.size(), &parser.vertices_data[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
}

void LineRenderer::Draw(std::vector<Line> lines, LineRenderInfo info)
{
	if (!initialized) {
		print_red("LineRenderer must be initialized before Draw()");
		return;
	}
	glBindVertexArray(vao);
	shader.Use();
	shader.setMat4("projection", info.projection);
	shader.setMat4("view", info.view);

	for (auto l : lines) {
		shader.setVec4("color", l.color);

		float dist = glm::distance(l.end, l.start);
		float vert_angle = -atan2(l.end.z-l.start.z, l.end.x-l.start.x);
		float horiz_angle = atan2(l.end.y - l.start.y,
			glm::distance(glm::vec2{ l.start.x, l.start.z }, glm::vec2{l.end.x, l.end.z}));

		glm::mat4 model = glm::mat4();
		model = glm::translate(model, l.start);
		model = glm::rotate(model, vert_angle, glm::vec3(0, 1, 0));
		model = glm::rotate(model, horiz_angle, glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(dist/2.f, l.thickness/140.f, l.thickness/140.f));
		model = glm::translate(model, glm::vec3(1.f, 0.f, 0.f));
		shader.setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}
