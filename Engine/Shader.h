#pragma once

#include "Globals.h"

class Shader
{
public:
	Shader()=default;
	Shader(cstr shader_name,
		cstr root = "../Resources/Shaders/",
		cstr vertex_extension = ".vert",
		cstr fragment_extension = ".frag");
	void Init(cstr shader_name,
		cstr root = "../Resources/Shaders/",
		cstr vertex_extension = ".vert",
		cstr fragment_extension = ".frag");

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
};
