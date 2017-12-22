#version 330 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	//vec4 world_pos = model * vec4(inPos, 1.0);
	vec4 view_pos = view * model * vec4(inPos, 1.0);
	//FragPos = world_pos.xyz;
FragPos = view_pos.xyz;
	Normal = transpose(inverse(mat3(view * model))) * inNormal;
	gl_Position = projection * view_pos;
}

