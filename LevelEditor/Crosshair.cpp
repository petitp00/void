#include "stdafx.h"
#include "Crosshair.h"

void Crosshair::Init(glm::vec2 screen_size)
{
	glEnable(GL_PROGRAM_POINT_SIZE);

	projection = glm::ortho(0.f, screen_size.x, screen_size.y, 0.f);
	shader.Init("crosshair");
	shader.Use();
	shader.setMat4("projection", projection);
	shader.setMat4("view", glm::mat4());
	shader.setVec4("color", { 1.0f, 0.2f, 0.2f, 1.0f });
	glm::mat4 model = glm::mat4();
	model = glm::translate(model, glm::vec3(screen_size.x / 2.f, screen_size.y / 2.f, 0.f));
	shader.setMat4("model", model);

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

void Crosshair::Draw()
{
	glBindVertexArray(vao);
	shader.Use();

	glDrawArrays(GL_POINTS, 0, 3);
}

void Crosshair::ChangeFramebufferSize(glm::vec2 new_size)
{
	projection = glm::ortho(0.f, new_size.x, new_size.y, 0.f);
	shader.Use();
	shader.setMat4("projection", projection);
	glm::mat4 model = glm::mat4();
	//model = glm::translate(model, glm::vec3(new_size.x / 2.f, new_size.y / 2.f, 0.f));
	shader.setMat4("model", model);
}
