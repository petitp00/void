#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D ssao_input;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(ssao_input, 0));
	float result = 0.0;
	for (int x = -2; x != 2; ++x) {
		for (int y = -2; y != 2; ++y) {
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssao_input, TexCoords + offset).r;
		}
	}
	float res = result / (4.0*4.0);
	FragColor = vec4(res, res, res, 1.0);
}
