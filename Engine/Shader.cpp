#include "stdafx.h"
#include "Shader.h"
#include "ConsoleColors.h"

Shader::Shader(std::string shader_name, std::string root, std::string vertex_extension, std::string fragment_extension)
{
	Init(shader_name, root, vertex_extension, fragment_extension);
}

void Shader::Init(std::string shader_name, std::string root, std::string vertex_extension, std::string fragment_extension)
{
	this->shader_name = shader_name;

	std::string vertex_code, fragment_code;
	std::ifstream vfile, ffile;
	vfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	ffile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vfile.open(root + shader_name + vertex_extension);
		ffile.open(root + shader_name + fragment_extension);
		std::stringstream vss, fss;
		vss << vfile.rdbuf();
		fss << ffile.rdbuf();
		vfile.close();
		ffile.close();
		vertex_code = vss.str();
		fragment_code = fss.str();
	}
	catch (std::ifstream::failure e) {
		print_red("ERROR: shader file could not be read");
		print_red((root + shader_name).c_str());
	}
	cstr vert_code = vertex_code.c_str();
	cstr frag_code = fragment_code.c_str();
	uint vert, frag;

	vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vert_code, NULL);
	glCompileShader(vert);
	CheckCompileErrors(vert, "VERTEX");

	frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &frag_code, NULL);
	glCompileShader(frag);
	CheckCompileErrors(frag, "FRAGMENT");

	id = glCreateProgram();
	glAttachShader(id, vert);
	glAttachShader(id, frag);
	glLinkProgram(id);
	CheckCompileErrors(id, "PROGRAM");

	glDeleteShader(vert);
	glDeleteShader(frag);
	
	ready_to_use = true;
}

void Shader::Use()
{
	if (!ready_to_use) {
		print_red("Shader is not ready to use. Init() must be called.");
		return;
	}
	glUseProgram(id);
}

void Shader::setBool(cstr name, bool value) {
	int loc = glGetUniformLocation(id, name);
	if (loc == -1)
		std::cout << red << "Location of uniform " << yellow << name << red << " was not found in shader "<< yellow << shader_name << '\n' << white;
	glUniform1i(loc, (int)value);
}
void Shader::setInt(cstr name, int value) {
	int loc = glGetUniformLocation(id, name);
	if (loc == -1)
		std::cout << red << "Location of uniform " << yellow << name << red << " was not found in shader "<< yellow << shader_name << '\n' << white;
	glUniform1i(loc, value);
}
void Shader::setFloat(cstr name, float value) {
	int loc = glGetUniformLocation(id, name);
	if (loc == -1)
		std::cout << red << "Location of uniform " << yellow << name << red << " was not found in shader "<< yellow << shader_name << '\n' << white;
	glUniform1f(loc, value);
}
void Shader::setVec2(cstr name, glm::vec2 value) {
	int loc = glGetUniformLocation(id, name);
	if (loc == -1)
		std::cout << red << "Location of uniform " << yellow << name << red << " was not found in shader "<< yellow << shader_name << '\n' << white;
	glUniform2f(loc, value.x, value.y);
}
void Shader::setVec3(cstr name, glm::vec3 value) {
	int loc = glGetUniformLocation(id, name);
	if (loc == -1)
		std::cout << red << "Location of uniform " << yellow << name << red << " was not found in shader "<< yellow << shader_name << '\n' << white;
	glUniform3f(loc, value.x, value.y, value.z);
}
void Shader::setVec4(cstr name, glm::vec4 value) {
	int loc = glGetUniformLocation(id, name);
	if (loc == -1)
		std::cout << red << "Location of uniform " << yellow << name << red << " was not found in shader "<< yellow << shader_name << '\n' << white;
	glUniform4f(loc, value.r, value.g, value.b, value.a);
}
void Shader::setMat4(cstr name, glm::mat4 value) {
	int loc = glGetUniformLocation(id, name);
	if (loc == -1)
		std::cout << red << "Location of uniform " << yellow << name << red << " was not found in shader "<< yellow << shader_name << '\n' << white;
	glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]);
}

void Shader::CheckCompileErrors(uint shader, std::string type)
{
	int success;
	char infoLog[512];

	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << red << "ERROR::SHADER_COMPILE::" << type << '\n' << yellow;
			std::cout << infoLog << '\n' << white;
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << red << "ERROR::SHADER_LINK::" << type << '\n' << yellow;
			std::cout << infoLog << '\n' << white;
		}
	}
}
