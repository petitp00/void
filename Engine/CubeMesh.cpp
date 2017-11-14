#include "stdafx.h"
#include "CubeMesh.h"
#include "ObjParser.h"

Shader CubeMesh::shader;
Shader CubeMesh::gbuf_shader;
Shader CubeMesh::light_pass_shader;
uint CubeMesh::vao_cube;
uint CubeMesh::vao_hcube;
uint CubeMesh::quad_vao;
uint CubeMesh::gbuf;
uint CubeMesh::gpos;
uint CubeMesh::gnorm;

void CubeMesh::Init(glm::ivec2 framebuffer_size)
{
	shader.Init("cube");
	//gbuf_shader.Init("cube", RESOURCES_FOLDER + "Shaders/", ".vert", "_gbuf.frag");
	gbuf_shader.Init("cube_gbuf");
	light_pass_shader.Init("cube_lightpass");

	light_pass_shader.Use();
	light_pass_shader.setInt("gPosition", 0);
	light_pass_shader.setInt("gNormal", 1);

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

	float quad_data[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	uint vbo_quad;
	glGenBuffers(1, &vbo_quad);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);

	// G Buffer
	glGenFramebuffers(1, &gbuf);
	glBindFramebuffer(GL_FRAMEBUFFER, gbuf);

	glGenTextures(1, &gpos);
	glBindTexture(GL_TEXTURE_2D, gpos);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gpos, 0);

	glGenTextures(1, &gnorm);
	glBindTexture(GL_TEXTURE_2D, gnorm);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gnorm, 0);

	uint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	
	uint rbo_depth;
	glGenRenderbuffers(1, &rbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_size.x, framebuffer_size.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		print_red("Framebuffer not complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
void CubeMesh::DrawCubesDeferred(std::vector<Cube> cubes, RenderInfo info)
{
	glBindFramebuffer(GL_FRAMEBUFFER, gbuf);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gbuf_shader.Use();
		gbuf_shader.setMat4("projection", info.proj);
		gbuf_shader.setMat4("view", info.view);

		glBindVertexArray(vao_cube);
		for (auto c : cubes) {
			glm::mat4 model = glm::mat4();
			model = glm::translate(model, c.position);
			model = glm::scale(model, c.size);
			gbuf_shader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	light_pass_shader.Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gpos);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gnorm);

	Cube c;
	light_pass_shader.setInt("gPosition", 0);
	light_pass_shader.setInt("gNormal", 1);
	light_pass_shader.setVec3("viewPos", info.view_pos);
	light_pass_shader.setVec3("light.position", info.light.position);
	light_pass_shader.setVec3("light.ambient", info.light.ambient);
	light_pass_shader.setVec3("light.diffuse", info.light.diffuse);
	light_pass_shader.setVec3("light.specular", info.light.specular);
	light_pass_shader.setBool("light.directional", info.light.directional);
	light_pass_shader.setFloat("light.constant", info.light.constant);
	light_pass_shader.setFloat("light.linear", info.light.linear);
	light_pass_shader.setFloat("light.quadratic", info.light.quadratic);
	light_pass_shader.setVec4("mat.diffuse", c.mat.diffuse);
	light_pass_shader.setVec4("mat.specular", c.mat.specular);
	light_pass_shader.setFloat("mat.specular_strength", c.mat.specular_strength);

	glBindVertexArray(quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuf);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(0, 0, info.framebuffer_size.x, info.framebuffer_size.y, 0, 0, info.framebuffer_size.x, info.framebuffer_size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
