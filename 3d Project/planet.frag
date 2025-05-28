#version 460 core

out vec4 fragColor;

in vec3 vertColor;
in vec3 normal;
in vec3 crntPos;

in vec2 texCoord;
uniform sampler2D diffuse0;
uniform sampler2D specular0;

uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

uniform vec3 planetPos;


float ambient = 0.15;

vec4 pointLight(){

	// use inverse square law to calculate intensity
	vec3 lightVec = lightPos - crntPos;
	float dist = length(lightVec);
	float a = 0.5;
	float b = 0.01;
	float intensity = 1.0 / (a * dist * dist + b * dist + 1.0);
	
	// diffuse lighting
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0);

	// specular lighting
	float specularLight = 0.50;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0), 16);
	float specular = specAmount * specularLight;

	return texture(diffuse0, texCoord) * vec4(vertColor, 1.0) * lightColor * (diffuse * intensity + ambient) + (texture(specular0, texCoord).r * specular * intensity);
};

vec4 directionalLight(){
	
	// easy to understand
	vec3 lightDirection = (lightPos - crntPos);
	
	// diffuse lighting
	lightDirection = normalize(lightDirection);
	float diffuse = max(dot(normal, lightDirection), 0.0);

	// specular lighting
	float specularLight = 0.50;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0), 16);
	float specular = specAmount * specularLight * 0.2;

	return vec4(vertColor, 1.0) * lightColor * (diffuse + ambient) + specular;

};

vec4 spotLight(){
	
	// inner and outer cones written in cos to save comp pow
	float outerCone = 0.9;
	float innerCone = 0.95;

	// diffuse lighting
	vec3 lightDirection = normalize(lightPos - crntPos);
	float diffuse = max(dot(normal, lightDirection), 0.0);

	// specular lighting
	float specularLight = 0.50;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0), 16);
	float specular = specAmount * specularLight;

	float angle = dot(vec3(0.0, -1.0, 0.0), -lightDirection);
	float intensity = clamp((angle - outerCone) / (innerCone - outerCone), 0.0, 1.0);

	return texture(diffuse0, texCoord) * vec4(vertColor, 1.0) * lightColor * (diffuse * intensity + ambient) + (texture(specular0, texCoord).r * specular * intensity);

}

void main(){

	// Get steepness
	float steepness = dot(normal, normalize(crntPos - planetPos));
	steepness = max(steepness, 0.0);

	vec4 steepnessCol;
	if(steepness < 0.97){
		steepnessCol = vec4(0.7);
	}
	else{
		steepnessCol = vec4(1.0);
	}

	// output final color
	fragColor = steepnessCol * directionalLight();

};
