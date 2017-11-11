#version 330 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in mat4 inInstanceMatrix;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * inInstanceMatrix * vec4(inPos, 1.0f);
}

