#version 460 core


in vec2 texCoords;
in vec3 atmosphereCentre;

out vec4 fragColor;

uniform sampler2D screenTexture;




void main(){

	fragColor = texture(screenTexture, texCoords);
} 