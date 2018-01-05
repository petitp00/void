#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

uniform vec2 screen_size;
uniform mat4 projection;

int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

void main()
{
	vec2 noiseScale = screen_size*0.25;

	vec3 fragPos = texture(gPosition, TexCoords).xyz;
	vec3 normal = normalize(texture(gNormal, TexCoords).xyz);
	if (normal == vec3(0,0,0)) {
		discard;
	}
	vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i != kernelSize; ++i) {
		vec3 sample = TBN * samples[i];
		sample = fragPos + sample * radius;
		
		vec4 offset = vec4(sample, 1.0);
		offset = projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		vec4 samplePos = texture(gPosition, offset.xy);
		if (samplePos == vec4(0,0,0,1)) continue;

		float sampleDepth = texture(gPosition, offset.xy).z;


		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	occlusion = pow(occlusion, 4);
	FragColor = vec4(occlusion, occlusion, occlusion, 1.0);
	
}
