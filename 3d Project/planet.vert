#version 460 core

// The part of each vertex array in the vertexarrayobject
// The VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
// 0 says what "location" the info can be retrieved and is set at. Position is at location 0 for example
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTex;

// Output color for frag
out vec3 crntPos;
out vec3 Normal;
out vec3 vertColor;
out vec2 texCoord;

float shift = 0.5f;

uniform mat4 camMatrix;
uniform mat4 model;
uniform mat4 translation;
uniform mat4 rotation;
uniform mat4 scale;


void main(){

	// Get current position
	crntPos = vec3(model * translation * rotation * scale * vec4(aPos, 1.0f));


	vertColor = aColor;

	// Rotate normal
	Normal = vec3(rotation * vec4(aNormal, 1.0f));

	texCoord = mat2(0.0, -1.0f, 1.0, 0.0) * aTex;
	
	// Multiply the position by the cameras matrix
	gl_Position = camMatrix * vec4(crntPos, 1.0);
};