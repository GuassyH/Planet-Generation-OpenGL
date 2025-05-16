#version 460 core

out vec4 fragColor;

in vec3 vertColor;
in vec3 Normal;
in vec3 crntPos;


in vec2 texCoord;
uniform sampler2D diffuse0;
uniform sampler2D specular0;

uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;



float ambient = 0.15f;

vec4 pointLight(){

	// use inverse square law to calculate intensity
	vec3 lightVec = lightPos - crntPos;
	float dist = length(lightVec);
	float a = 0.5f;
	float b = 0.01f;
	float intensity = 1.0f / (a * dist * dist + b * dist + 1.0f);
	
	// diffuse lighting
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;

	return texture(diffuse0, texCoord) * vec4(vertColor, 1.0f) * lightColor * (diffuse * intensity + ambient) + (texture(specular0, texCoord).r * specular * intensity);
};

vec4 directionalLight(){
	
	// easy to understand
	vec3 lightDirection = (lightPos - crntPos);
	
	// diffuse lighting
	lightDirection = normalize(lightDirection);
	vec3 normal = normalize(Normal);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight * 0.2f;

	return vec4(vertColor, 1.0f) * lightColor * (diffuse + ambient) + specular;

};

vec4 spotLight(){
	
	// inner and outer cones written in cos to save comp pow
	float outerCone = 0.9f;
	float innerCone = 0.95f;

	// diffuse lighting
	vec3 lightDirection = normalize(lightPos - crntPos);
	vec3 normal = normalize(Normal);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;

	float angle = dot(vec3(0.0f, -1.0f, 0.0f), -lightDirection);
	float intensity = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f);

	return texture(diffuse0, texCoord) * vec4(vertColor, 1.0f) * lightColor * (diffuse * intensity + ambient) + (texture(specular0, texCoord).r * specular * intensity);

}

void main(){
	// output final color
	float steepness = clamp(dot(Normal, normalize(crntPos)), 0.0f, 1.0f); 
	// fragColor = steepness * directionalLight();
	fragColor = directionalLight();
};
