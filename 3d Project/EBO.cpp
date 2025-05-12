#include "EBO.h"

EBO::EBO(std::vector<GLuint>& indices) {

	// Create the buffer, bind the GL_ELEMENT_ARRAY_BUFFER to this EBO's ID, Set the data
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}


void EBO::Bind() {
	// Bind the GL_ELEMENT_ARRAY_BUFFER to ID
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void EBO::Unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO::Delete() {
	glDeleteBuffers(1, &ID);
}

