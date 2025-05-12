#include "VBO.h"

VBO::VBO(std::vector<Vertex>& vertices) {

	// Create the buffer, bind the GL_ARRAY_BUFFET to this VBO's ID, Set the data
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}


void VBO::Bind() {
	// Bind the GL_ARRAY_BUFFER to ID
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete() {
	glDeleteBuffers(1, &ID);
}

