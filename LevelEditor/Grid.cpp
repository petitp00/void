#include "stdafx.h"
#include "Grid.h"

#include "../Engine/LineRenderer.h"
#include "../Engine/ObjParser.h"

Grid::Grid(int size, float scale, glm::vec4 color, float thickness) : size(size), color(color)
{
	Line* lines = new Line[2*size];
	for (int x = -size / 2; x != size / 2; ++x) {
		Line l;
		l.start = glm::vec3(x*scale, 0, -size / 2 * scale);
		l.end = glm::vec3(x*scale, 0, size / 2 * scale);
		l.color = color;
		l.thickness = thickness;
		lines[x + size / 2] = l;
	}
	for (int z = -size / 2; z != size / 2; ++z) {
		Line l;
		l.start = glm::vec3(-size/2 * scale, 0, z*scale);
		l.end = glm::vec3(size/2 * scale, 0, z*scale);
		l.color = color;
		l.thickness = thickness;
		lines[size + z + size / 2] = l;
	}

	shader.Init("instanced_primitive");
	ObjParser parser("cubeline.obj");
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	uint vbo, vbo_instance;
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &vbo_instance);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * parser.vertices_data.size(), &parser.vertices_data[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glm::mat4* models = new glm::mat4[2*size];
	for (int i = 0; i != 2*size; ++i) {
		auto l = lines[i];
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
		models[i] = model;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_instance);
	glBufferData(GL_ARRAY_BUFFER, 2*size * sizeof(glm::mat4), models, GL_STATIC_DRAW);
	auto vec4_size = sizeof(glm::vec4);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(vec4_size));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(2 * vec4_size));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(3 * vec4_size));

	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);

	delete[] lines;
	delete[] models;
}

void Grid::Draw(LineRenderInfo info)
{
	shader.Use();
	shader.setMat4("projection", info.projection);
	shader.setMat4("view", info.view);
	shader.setVec4("color", color);
	glBindVertexArray(vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 2*size);
}
