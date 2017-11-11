#version 330 core
out vec4 FragColor;

in vec2 uv;

uniform sampler2D tex;

void main()
{
	vec4 color = texture(tex, uv);
	if (color.a < 0.5) discard;
	FragColor = color;
}

