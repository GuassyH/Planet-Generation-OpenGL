#define PYRAMID_CLASS_H

#include <GLFW/glfw3.h>
#include <glad/glad.h>

class Pyramid {
	public:
		GLfloat* vertices;
		GLuint* indices;
		Pyramid();
};