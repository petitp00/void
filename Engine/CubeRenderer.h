#pragma once

#include "Globals.h"
#include "Shader.h"

struct CubeMaterial {
	glm::vec4 diffuse = glm::vec4(1.f);
	glm::vec4 specular = glm::vec4(1.0f);
	float specular_strength = 2.f;
};

struct LightInfo {
	glm::vec3 position; // direction if directional is true
	glm::vec3 ambient = glm::vec3(0.0f);
	glm::vec3 diffuse = glm::vec3(1.f);
	glm::vec3 specular = glm::vec3(0.f);
	bool directional = false; // set to false for point light
	float constant = 1.f;
	float linear = 0.09f;
	float quadratic = 0.032f;
};

struct RenderInfo {
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec3 view_pos;
	LightInfo light;
	glm::ivec2 framebuffer_size;
};

struct Cube {
	glm::vec3 position;
	glm::vec3 size;
	CubeMaterial mat;
};

struct HalfCube { // sometimes called hcube in code
	glm::vec3 position;
	glm::vec3 size = glm::vec3(1.f); // before rotation [x: length, y: height, z: width]
	glm::vec2 rotation = { 1, 0 }; // x or z can be 1 or -1
	CubeMaterial mat;
};

class CubeRenderer
{
public:
	CubeRenderer() = delete;
	static void Init(glm::ivec2 framebuffer_size);
	static void ChangeFramebuffer_size(glm::ivec2 frambuffer_size);

	static void AddCubesToDraw(std::vector<Cube> _cubes, std::vector<HalfCube> _hcubes);
	static void RenderFrame(RenderInfo info); // clears cubes and hcubes

private:
	static uint gbuf;
	static uint gpos, gnorm;
	static uint vao_cube, vao_hcube, vao_quad;
	static Shader gbuf_shader, light_pass_shader;

	static std::vector<Cube> cubes;
	static std::vector<HalfCube> hcubes;
};

