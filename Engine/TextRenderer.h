#pragma once

#include "Globals.h"
#include "Shader.h"

#include <map>

struct Character {
	uint		TextureID;
	glm::ivec2	Size;	 // size of glyph
	glm::ivec2	Bearing; // offset from baseline to left/top of glyph
	uint		Advance; // offset to advance to next glyph
};

struct TextToWorldRenderInfo {
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 view_pos;
};

class TextRenderer
{
public:
	TextRenderer()=default;
	TextRenderer(glm::vec2 framebuffer_size, std::string font_filename, uint font_size, std::string fonts_folder = RESOURCES_FOLDER + "Fonts/");
	void Init(glm::vec2 framebuffer_size, std::string font_filename, uint font_size, std::string fonts_folder = RESOURCES_FOLDER + "Fonts/");
	void ChangeFramebufferSize(glm::vec2 new_size);
	void RenderText(std::string text, glm::vec2 pos, float scale, glm::vec3 color = glm::vec3(0.f));
	void RenderTextToWorld(std::string text, glm::vec3 pos, float scale, glm::vec3 color, TextToWorldRenderInfo ttwri);

	std::map<char, Character> characters;
	Shader shader;
	Shader shader_fbo, shader_billboard;

private:
	bool ready_to_render = false; // is set to true in Init
	uint vao, vbo;
	uint vao_billboard, fbo, fbo_texture;
	glm::vec2 framebuffer_size;
};

