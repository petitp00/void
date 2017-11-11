#version 330 core
layout (location = 0) in vec2 inPos;

out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 low_corner;

void main()
{

	uv = inPos;
	uv.y = abs(uv.y-1); // 0->1, 1->0
	uv *= vec2(low_corner.x / 1024, -low_corner.y/1024);
	
	gl_Position = projection * view * model * vec4(inPos - vec2(0.5, 0.5), 0, 1.0f);
}

