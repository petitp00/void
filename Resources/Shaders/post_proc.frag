#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTex;

// 0.5 results in a cercle fitting in the screen
const float vignette_radius = 1.2;
// between 0.0 and 1.0
const float vignette_softness = 0.85;

void main()
{
	vec2 screenSize = textureSize(screenTex, 0);
	vec3 col = texture(screenTex, TexCoords).rgb;

	vec2 center = TexCoords - vec2(0.5, 0.5);
	center.x *= screenSize.x / screenSize.y;
	
	float vignette = smoothstep(
		vignette_radius,
		vignette_radius-vignette_softness,
		length(center)
	);

	col = mix(col, col * vignette, 0.5);

	FragColor = vec4(col, 1);
}
