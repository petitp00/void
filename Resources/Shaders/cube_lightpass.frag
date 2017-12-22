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
uniform sampler2D gColor;
uniform sampler2D ssao;

uniform Material mat;
uniform Light light;

void main()
{

	vec4 color = texture(gColor, TexCoords);
	//color = vec4(1,1,1,1);
	
	vec3 FragPos = /*viewPos -*/ texture(gPosition, TexCoords).rgb;
	vec3 norm = normalize(texture(gNormal, TexCoords)).rgb;
	float ssao_val = texture(ssao, TexCoords).r;
	if (norm == vec3(0,0,0)) { discard; }
	if (norm == vec3(normalize(vec4(1,1,1,1)))) { discard; }


	vec3 light_dir;
	if (!light.directional) {
		light_dir = normalize(light.position - FragPos);
	}
	else {
		//light_dir = normalize(-light.position);
	}

	vec3 ambient = light.ambient * color.rgb * ssao_val;
	//vec3 ambient = light.ambient * mat.diffuse.rgb;
	vec3 lighting = ambient;
	vec3 view_dir = normalize(-FragPos);

	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff * color.rgb;

	vec3 reflect_dir = reflect(-light_dir, norm);	
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), mat.specular_strength);
	vec3 specular = light.specular * spec * mat.specular.rgb;

	if (!light.directional) {
		float dist = length(-FragPos);
		float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
	}

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
}

