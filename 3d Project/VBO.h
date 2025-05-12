#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
};

struct ComputeVertex {
	glm::vec4 position;
	glm::vec4 normal;
	glm::vec4 color;
	glm::vec4 texUV;
};


class VBO {
	public:
		GLuint ID;
		VBO(std::vector <Vertex>& vertices);

		void Bind();
		void Unbind();
		void Delete();

};


#endif