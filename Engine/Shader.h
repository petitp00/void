#pragma once

#include "Globals.h"

class Shader
{
public:
	Shader()=default;
	Shader(std::string shader_name,
		std::string root = RESOURCES_FOLDER + "Shaders/",
		std::string vertex_extension = ".vert",
		std::string fragment_extension = ".frag");
	void Init(std::string shader_name,
		std::string root = RESOURCES_FOLDER + "Shaders/",
		std::string vertex_extension = ".vert",
		std::string fragment_extension = ".frag");

	void Use();

	void setBool(cstr name, bool value);
	void setInt(cstr name, int value);
	void setFloat(cstr name, float value);
	void setVec2(cstr name, glm::vec2 value);
	void setVec3(cstr name, glm::vec3 value);
	void setVec4(cstr name, glm::vec4 value);
	void setMat4(cstr name, glm::mat4 value);

	uint id;
	
private:
	void CheckCompileErrors(uint shader, std::string type);
	bool ready_to_use = false; // is set to true in init
	std::string shader_name;
};
