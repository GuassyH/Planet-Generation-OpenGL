#version 460 core

out vec4 fragColor;
in vec3 vertColor;

uniform vec4 lightColor;

void main(){
	fragColor = lightColor;
};