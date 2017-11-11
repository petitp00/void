#pragma once

#include "../Engine/Shader.h"

class Crosshair
{
public:
	Crosshair()=default;
	void Init(glm::vec2 screen_size);
	void Draw();
	void ChangeFramebufferSize(glm::vec2 new_size);

private:
	uint vao;
	glm::mat4 projection;
	Shader shader;
};

