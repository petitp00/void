#version 330 core
out vec4 FragColor;

uniform vec4 color;

void main()
{
	vec4 col = color;
	vec2 coord = 2 * gl_PointCoord - 1;
	float d = dot(coord, coord);

	if (d > 0.9) {
		col.a = 10*(1-d);
	}
	if (d > 1) {
		discard;
	}
	FragColor = col;
}

