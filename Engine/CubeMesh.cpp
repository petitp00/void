#include "stdafx.h"
#include "CubeMesh.h"
#include "ObjParser.h"

Shader CubeMesh::shader;
uint CubeMesh::vao_cube;
uint CubeMesh::vao_hcube;

void CubeMesh::Init()
{
	shader.Init("cube");

	// Cube
	ObjParser parser("cube.obj");

	glGenVertexArrays(1, &vao_cube);
	glBindVertexArray(vao_cube);

	uint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * parser.data.size(), &parser.data[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// HCube
	parser.ParseFile("halfcube.obj");

	glGenVertexArrays(1, &vao_hcube);
	glBindVertexArray(vao_hcube);

	uint vbo_hcube;
	glGenBuffers(1, &vbo_hcube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_hcube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * parser.data.size(), &parser.data[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void CubeMesh::DrawCubes(std::vector<Cube> cubes, RenderInfo info)
{
	glBindVertexArray(vao_cube);
	shader.Use();

	shader.setMat4("projection", info.proj);
	shader.setMat4("view", info.view);

	shader.setVec3("viewPos", info.view_pos);

	shader.setVec3("light.position", info.light.position);
	shader.setVec3("light.ambient", info.light.ambient);
	shader.setVec3("light.diffuse", info.light.diffuse);
	shader.setVec3("light.specular", info.light.specular);
	shader.setBool("light.directional", info.light.directional);
	shader.setFloat("light.constant", info.light.constant);
	shader.setFloat("light.linear", info.light.linear);
	shader.setFloat("light.quadratic", info.light.quadratic);

	for (auto c : cubes) {
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, c.position);
		model = glm::scale(model, c.size);
		shader.setMat4("model", model);
		shader.setVec4("mat.diffuse", c.mat.diffuse);
		shader.setVec4("mat.specular", c.mat.specular);
		shader.setFloat("mat.specular_strength", c.mat.specular_strength);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void CubeMesh::DrawHalfCubes(std::vector<HalfCube> hcubes, RenderInfo info)
{
	glBindVertexArray(vao_hcube);
	shader.Use();

	shader.setMat4("projection", info.proj);
	shader.setMat4("view", info.view);

	shader.setVec3("viewPos", info.view_pos);

	shader.setVec3("light.position", info.light.position);
	shader.setVec3("light.ambient", info.light.ambient);
	shader.setVec3("light.diffuse", info.light.diffuse);
	shader.setVec3("light.specular", info.light.specular);
	shader.setBool("light.directional", info.light.directional);
	shader.setFloat("light.constant", info.light.constant);
	shader.setFloat("light.linear", info.light.linear);
	shader.setFloat("light.quadratic", info.light.quadratic);

	for (auto c : hcubes) {

		float angle_val = 0;

		if (c.rotation.x == -1) {
			angle_val = glm::radians(180.f);
		}
		else if (c.rotation.y == -1) {
			angle_val = glm::radians(90.f);
		}
		else if (c.rotation.y == 1) {
			angle_val = glm::radians(270.f);
		}

		glm::mat4 model = glm::mat4();
		model = glm::translate(model, c.position);
		model = glm::rotate(model, angle_val, glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, c.size);
		shader.setMat4("model", model);
		shader.setVec4("mat.diffuse", c.mat.diffuse);
		shader.setVec4("mat.specular", c.mat.specular);
		shader.setFloat("mat.specular_strength", c.mat.specular_strength);

		glDrawArrays(GL_TRIANGLES, 0, 8 * 3);
	}
}
