#include "stdafx.h"
#include "CubeRenderer.h"
#include "ObjParser.h"

#include <random>

std::vector<Cube> CubeRenderer::cubes;
std::vector<HalfCube> CubeRenderer::hcubes;

uint CubeRenderer::vao_cube;
uint CubeRenderer::vao_hcube;
uint CubeRenderer::vao_quad;

Shader CubeRenderer::gbuf_shader;
uint CubeRenderer::gbuf;
uint CubeRenderer::gpos;
uint CubeRenderer::gnorm;
uint CubeRenderer::gcolor;

Shader CubeRenderer::ssao_shader;
uint CubeRenderer::ssao_fbo;
uint CubeRenderer::ssao_col_buf;
uint CubeRenderer::noise_texture;

Shader CubeRenderer::ssao_blur_shader;
uint CubeRenderer::ssao_blur_fbo;
uint CubeRenderer::ssao_blur_col_buf;

Shader CubeRenderer::light_pass_shader;
uint CubeRenderer::light_pass_fbo;
uint CubeRenderer::light_pass_col_buf;

Shader CubeRenderer::fxaa_shader;
uint CubeRenderer::fxaa_fbo;
uint CubeRenderer::fxaa_col_buf;


void CubeRenderer::Init(glm::ivec2 framebuffer_size)
{
	gbuf_shader.Init("cube_gbuf");
	light_pass_shader.Init("cube_lightpass");
	ssao_shader.Init("cube_ssao");
	ssao_blur_shader.Init("cube_ssao_blur");
	fxaa_shader.Init("fxaa");

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

	// Quad
	float quad_data[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &vao_quad);
	glBindVertexArray(vao_quad);

	uint vbo_quad;
	glGenBuffers(1, &vbo_quad);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);


	auto lerp = [](float a, float b, float f) {
		return a + f * (b - a);
	};

	ssao_shader.Use();
	std::vector<glm::vec3> ssao_kernel;
	std::uniform_real_distribution<float> random_floats(0.0, 1.0);
	std::default_random_engine generator;
	for (uint i = 0; i != 64; ++i) {
		glm::vec3 sample(
			random_floats(generator) * 2.f - 1.f,
			random_floats(generator) * 2.f - 1.f,
			random_floats(generator)
		);
		sample = glm::normalize(sample);
		sample *= random_floats(generator);
		float scale = (float)i / 64.f;
		scale = lerp(0.1f, 1.0f, scale*scale);
		sample *= scale;
		ssao_kernel.push_back(sample);
		std::string uni = std::string("samples[" + std::to_string(i) + "]");
		ssao_shader.setVec3(uni.c_str(), ssao_kernel[i]);
	}

	std::vector<glm::vec3> ssao_noise;
	for (uint i = 0; i != 16; ++i) {
		glm::vec3 noise(
			random_floats(generator) * 2.f - 1.f,
			random_floats(generator) * 2.f - 1.f,
			0.f
		);
		ssao_noise.push_back(noise);
	}

	glGenTextures(1, &noise_texture);
	glBindTexture(GL_TEXTURE_2D, noise_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssao_noise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

	InitFramebuffers(framebuffer_size);

}

void CubeRenderer::ChangeFramebufferSize(glm::ivec2 framebuffer_size)
{
	InitFramebuffers(framebuffer_size);
}

void CubeRenderer::AddCubesToDraw(std::vector<Cube> _cubes, std::vector<HalfCube> _hcubes)
{
	cubes.insert(cubes.end(), _cubes.begin(), _cubes.end());
	hcubes.insert(hcubes.end(), _hcubes.begin(), _hcubes.end());
}

void CubeRenderer::RenderFrame(RenderInfo info, uint fbo)
{
	glBindFramebuffer(GL_FRAMEBUFFER, gbuf);
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gbuf_shader.Use();
		gbuf_shader.setMat4("projection", info.proj);
		gbuf_shader.setMat4("view", info.view);

		glBindVertexArray(vao_cube);
		for (auto c : cubes) {
			glm::mat4 model = glm::translate(glm::mat4(), c.position);
			model = glm::scale(model, c.size);
			gbuf_shader.setMat4("model", model);
			gbuf_shader.setVec3("color", c.mat.diffuse);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		cubes.clear();
		glBindVertexArray(vao_hcube);
		for (auto c : hcubes) {
			float angle_val = 0;
			if (c.rotation.x == -1)			angle_val = glm::radians(180.f);
			else if (c.rotation.y == -1)	angle_val = glm::radians(90.f);
			else if (c.rotation.y == 1)		angle_val = glm::radians(270.f);
			glm::mat4 model = glm::translate(glm::mat4(), c.position);
			model = glm::rotate(model, angle_val, glm::vec3(0.f, 1.f, 0.f));
			model = glm::scale(model, c.size);
			gbuf_shader.setMat4("model", model);
			gbuf_shader.setVec3("color", c.mat.diffuse);
			glDrawArrays(GL_TRIANGLES, 0, 8 * 3);
		}
		hcubes.clear();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gpos);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gnorm);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noise_texture);
		ssao_shader.Use();
		ssao_shader.setInt("gPosition", 0);
		ssao_shader.setInt("gNormal", 1);
		ssao_shader.setInt("texNoise", 2);
		ssao_shader.setMat4("projection", info.proj);
		ssao_shader.setVec2("screen_size", info.framebuffer_size);

		glBindVertexArray(vao_quad);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo);
	{
		glClear(GL_COLOR_BUFFER_BIT);
		ssao_blur_shader.Use();
		ssao_blur_shader.setInt("ssao_input", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssao_col_buf);

		glBindVertexArray(vao_quad);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, light_pass_fbo);
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		light_pass_shader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gpos);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gnorm);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gcolor);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ssao_blur_col_buf);

		Cube c;
		light_pass_shader.setInt("gPosition", 0);
		light_pass_shader.setInt("gNormal", 1);
		light_pass_shader.setInt("gColor", 2);
		light_pass_shader.setInt("ssao", 3);
		light_pass_shader.setVec3("light.position", glm::vec3(info.view * glm::vec4(info.light.position, 1)));
		light_pass_shader.setVec3("light.ambient", info.light.ambient);
		light_pass_shader.setVec3("light.diffuse", info.light.diffuse);
		light_pass_shader.setVec3("light.specular", info.light.specular);
		light_pass_shader.setBool("light.directional", info.light.directional);
		light_pass_shader.setFloat("light.constant", info.light.constant);
		light_pass_shader.setFloat("light.linear", info.light.linear);
		light_pass_shader.setFloat("light.quadratic", info.light.quadratic);
		//light_pass_shader.setVec4("mat.diffuse", c.mat.diffuse);
		light_pass_shader.setVec4("mat.specular", c.mat.specular);
		light_pass_shader.setFloat("mat.specular_strength", c.mat.specular_strength);

		glBindVertexArray(vao_quad);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, fxaa_fbo);
	{
		glClear(GL_COLOR_BUFFER_BIT);
		fxaa_shader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, light_pass_col_buf);
		fxaa_shader.setInt("screenTex", 0);
		fxaa_shader.setVec2("inverseScreenSize", glm::vec2(1.0 / info.framebuffer_size.x, 1.0 / info.framebuffer_size.y));

		glBindVertexArray(vao_quad);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fxaa_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBlitFramebuffer(0, 0, info.framebuffer_size.x, info.framebuffer_size.y, 0, 0, info.framebuffer_size.x, info.framebuffer_size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuf);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBlitFramebuffer(0, 0, info.framebuffer_size.x, info.framebuffer_size.y, 0, 0, info.framebuffer_size.x, info.framebuffer_size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void CubeRenderer::InitFramebuffers(glm::ivec2 framebuffer_size)
{
	// G Buffer
	{
		if (gbuf != 0) {
			glDeleteFramebuffers(1, &gbuf);
			uint textures[] = { gpos, gnorm, gcolor };
			glDeleteTextures(3, textures);
		}

		glGenFramebuffers(1, &gbuf);
		glBindFramebuffer(GL_FRAMEBUFFER, gbuf);

		glGenTextures(1, &gpos);
		glBindTexture(GL_TEXTURE_2D, gpos);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gpos, 0);

		glGenTextures(1, &gnorm);
		glBindTexture(GL_TEXTURE_2D, gnorm);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gnorm, 0);

		glGenTextures(1, &gcolor);
		glBindTexture(GL_TEXTURE_2D, gcolor);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gcolor, 0);

		uint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);

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

	// SSAO
	{
		if (ssao_fbo != 0) {
			uint fbos[] = { ssao_fbo, ssao_blur_fbo };
			glDeleteFramebuffers(2, fbos);
			uint textures[] = { ssao_col_buf, ssao_blur_col_buf };
			glDeleteTextures(2, textures);
		}

		glGenFramebuffers(1, &ssao_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
		glGenTextures(1, &ssao_col_buf);
		glBindTexture(GL_TEXTURE_2D, ssao_col_buf);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_col_buf, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			print_red("Framebuffer not complete!");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glGenFramebuffers(1, &ssao_blur_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo);
		glGenTextures(1, &ssao_blur_col_buf);
		glBindTexture(GL_TEXTURE_2D, ssao_blur_col_buf);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_blur_col_buf, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			print_red("Framebuffer not complete!");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Light pass
		if (light_pass_fbo != 0) {
			glDeleteFramebuffers(1, &light_pass_fbo);
			glDeleteTextures(1, &light_pass_col_buf);
		}

		glGenFramebuffers(1, &light_pass_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, light_pass_fbo);
		glGenTextures(1, &light_pass_col_buf);
		glBindTexture(GL_TEXTURE_2D, light_pass_col_buf);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light_pass_col_buf, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			print_red("Framebuffer not complete!");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// FXAA
	{
		if (fxaa_fbo != 0) {
			glDeleteFramebuffers(1, &fxaa_fbo);
			glDeleteTextures(1, &fxaa_col_buf);
		}

		glGenFramebuffers(1, &fxaa_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fxaa_fbo);
		glGenTextures(1, &fxaa_col_buf);
		glBindTexture(GL_TEXTURE_2D, fxaa_col_buf);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fxaa_col_buf, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			print_red("Framebuffer not complete!");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

