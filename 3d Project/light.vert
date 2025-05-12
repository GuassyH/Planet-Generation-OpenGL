#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 vertColor;

uniform mat4 camMatrix;
uniform mat4 model;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;

out vec3 crntPos;

void main(){
	crntPos = vec3(model * translation * rotation * scale * vec4(aPos, 1.0f));
	gl_Position = camMatrix * vec4(crntPos, 1.0);
};