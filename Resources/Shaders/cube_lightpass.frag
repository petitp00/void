#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

struct Material {
	vec4 diffuse;
	vec4 specular;
	float specular_strength;
};

struct Light {
	vec3 position;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	bool directional;
	float constant;
	float linear;
	float quadratic;
};

uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform vec3 viewPos;
uniform Material mat;
uniform Light light;

void main()
{
	vec3 FragPos = texture(gPosition, TexCoords).rgb;

	vec3 ambient = light.ambient * mat.diffuse.rgb;


	vec3 norm = normalize(texture(gNormal, TexCoords)).rgb;

	if (norm == vec3(0,0,0)) {
		discard;
	}

	vec3 light_dir;
	if (!light.directional) {
		light_dir = normalize(light.position - FragPos);
	}
	else {
		light_dir = normalize(-light.position);
	}
	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff * mat.diffuse.rgb;

	vec3 view_dir = normalize(viewPos - FragPos);
	vec3 reflect_dir = reflect(-light_dir, norm);	
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), mat.specular_strength);
	vec3 specular = light.specular * spec * mat.specular.rgb;

	if (!light.directional) {
		float dist = length(light.position - FragPos);
		float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
	}

	vec3 result = ambient + diffuse + specular;
//result = texture(gNormal, TexCoords).rgb;
//result = texture(gPosition, TexCoords).rgb;
	//result = vec3(TexCoords, 0);
//result = FragPos;
	FragColor = vec4(result, 1.0);
}

