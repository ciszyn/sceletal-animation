#version 330 core

in vec3 fragNormal;
in vec3 fragPos;
in vec2 fragUV;

// Ouput data
out vec4 color;

uniform vec3 lightPos;
uniform sampler2D myTextureSampler;

void main()
{
	vec3 lightColor = vec3(1, 1, 1);
	vec3 objectColor = texture(myTextureSampler, fragUV).rgb;
	vec3 ambient = 0.2 * objectColor;

	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(lightPos - fragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 result = (ambient + diffuse) * objectColor;

	color = vec4(result, 1.0);
}