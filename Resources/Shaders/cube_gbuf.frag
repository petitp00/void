#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 color;

void main()
{
	gPosition = vec4(FragPos, 1);
	gNormal = vec4(Normal, 1); // don't forget to normalize in lighting frag shader
	gColor = vec4(color, 1);

	//gPosition = FragPos;
	//gNormal = Normal;
}
