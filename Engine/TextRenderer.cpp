#include "stdafx.h"
#include "TextRenderer.h"
#include "ConsoleColors.h"

#include <ft2build.h>
#include FT_FREETYPE_H

TextRenderer::TextRenderer(glm::vec2 framebuffer_size, cstr font_filename, uint font_size, cstr fonts_folder)
{
	Init(framebuffer_size, font_filename, font_size, fonts_folder);
}

void TextRenderer::Init(glm::vec2 framebuffer_size, cstr font_filename, uint font_size, cstr fonts_folder)
{
	this->framebuffer_size = framebuffer_size;

	shader.Init("text");
	shader.Use();
	glm::mat4 proj = glm::ortho(0.f, framebuffer_size.x, framebuffer_size.y, 0.f);
	shader.setMat4("projection", proj);
	shader.setInt("text", 0);

	shader_fbo.Init("text_to_texture");
	shader_fbo.Use();
	glm::mat4 proj_fbo = glm::ortho(0.f, 1024.f, 1024.f, 0.f);
	//glm::mat4 proj_fbo = glm::ortho(0.f, framebuffer_size.x, framebuffer_size.y, 0.f);
	shader_fbo.setMat4("projection", proj_fbo);
	shader_fbo.setInt("text", 0);

	shader_billboard.Init("billboard");

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &fbo_texture);
	glBindTexture(GL_TEXTURE_2D, fbo_texture);
	// create an empty image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// set fbo_texture as our color attachement 0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_texture, 0);

	GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		print_red("ERROR: FBO is incomplete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &vao_billboard);
	glBindVertexArray(vao_billboard);
	const float data[] = {
		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f,
		0.f, 0.f,
		1.f, 1.f,
		0.f, 1.f
	};

	uint vbo_bb;
	glGenBuffers(1, &vbo_bb);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_bb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (sizeof(float)) * 2, (void*)0);

	characters.clear();
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cout << red << "ERROR::FREETYPE: Could not init FreeType Library" << white << '\n';
		return;
	}
	FT_Face face;
	std::string font_file = std::string(fonts_folder) + font_filename;
	if (FT_New_Face(ft, font_file.c_str(), 0, &face)) {
		std::cout << red << "ERROR::FREETYPE: Failed to load font \"" << font_file  << "\"." << white << '\n';
		return;
	}
	FT_Set_Pixel_Sizes(face, 0, font_size);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (unsigned char c = 0; c < 128; ++c) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << red << "ERROR:FREETYPE: Failed to load glyph: '" << c << "'." << white << '\n';
			continue;
		}
		uint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0, GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0, GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			uint(face->glyph->advance.x)
		};
		characters[c] = character;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	ready_to_render = true;
}

void TextRenderer::ChangeFrameBufferSize(glm::vec2 new_size)
{
	framebuffer_size = new_size;
	shader.Use();
	glm::mat4 proj = glm::ortho(0.f, new_size.x, new_size.y, 0.f);
	shader.setMat4("projection", proj);
}

void TextRenderer::RenderText(std::string text, glm::vec2 pos, float scale, glm::vec3 color)
{
	if (!ready_to_render) {
		print_red("TextRenderer is not ready to render. Init needs to be called");
		return;
	}
	shader.Use();
	shader.setVec3("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao);

	for (auto c = text.begin(); c != text.end(); ++c) {
		Character ch = characters[*c];

		float xx = pos.x + ch.Bearing.x * scale;
		float yy = pos.y + (characters['H'].Bearing.y - ch.Bearing.y) * scale;
		float ww = ch.Size.x * scale;
		float hh = ch.Size.y * scale;

		float vertices[6][4] = {
			{ xx,		yy + hh,	0.f, 1.f },
			{ xx + ww,	yy,			1.f, 0.f },
			{ xx,		yy,			0.f, 0.f },
			{ xx,		yy + hh,	0.f, 1.f },
			{ xx + ww,	yy + hh,	1.f, 1.f },
			{ xx + ww,	yy,			1.f, 0.f }
		};

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		pos.x += (ch.Advance >> 6) * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::RenderTextToWorld(std::string text, glm::vec3 pos, float scale, glm::vec3 color, TextToWorldRenderInfo ttwri)
{
	if (!ready_to_render) {
		print_red("TextRenderer is not ready to render. Init needs to be called");
		return;
	}


	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, 1024, 1024);

	GLfloat clear_color[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);

	shader_fbo.Use();
	shader_fbo.setVec3("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao);

	float maxyh = 0.f;
	glm::vec3 orig_pos = pos;
	pos = { 0,0,0 };

	for (auto c = text.begin(); c != text.end(); ++c) {
		Character ch = characters[*c];

		float xx = pos.x + ch.Bearing.x * scale;
		float yy = pos.y + (characters['H'].Bearing.y - ch.Bearing.y) * scale;
		float ww = ch.Size.x * scale;
		float hh = ch.Size.y * scale;

		float vertices[6][4] = {
			{ xx,		yy + hh,	0.f, 1.f },
			{ xx + ww,	yy,			1.f, 0.f },
			{ xx,		yy,			0.f, 0.f },
			{ xx,		yy + hh,	0.f, 1.f },
			{ xx + ww,	yy + hh,	1.f, 1.f },
			{ xx + ww,	yy,			1.f, 0.f }
		};

		if (maxyh < yy + hh) {
			maxyh = yy + hh;
		}

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		pos.x += (ch.Advance >> 6) * scale;
	}

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, (int)framebuffer_size.x, (int)framebuffer_size.y);

	glBindVertexArray(vao_billboard);
	shader_billboard.Use();
	shader_billboard.setMat4("projection", ttwri.projection);
	shader_billboard.setMat4("view", ttwri.view);
	shader_billboard.setVec2("low_corner", { pos.x, maxyh });
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbo_texture);
	shader_billboard.setInt("tex", 0);

	float vert_angle = glm::acos(glm::dot(
		glm::vec2(0, 1), glm::normalize(
			glm::vec2(
				ttwri.view_pos.x - orig_pos.x,
				ttwri.view_pos.z - orig_pos.z
	))));

	if (ttwri.view_pos.x - orig_pos.x < 0) {
		vert_angle = -vert_angle;
	}
	float dist = glm::distance(ttwri.view_pos, orig_pos);

	glm::mat4 model = glm::mat4();
	model =	glm::translate(model, orig_pos-glm::vec3(0, 0.03*dist, 0));
	model = glm::rotate(model, vert_angle, { 0, 1, 0 });
	model = glm::scale(model, { pos.x/1024.f, maxyh/1024.f, 1.f });
	model = glm::scale(model, 0.4f * glm::vec3(dist));

	shader_billboard.setMat4("model", model);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
