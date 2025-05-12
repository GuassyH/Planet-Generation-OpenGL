#ifndef SPHERE_SIDE_CLASS_H
#define SPHERE_SIDE_CLASS_H

#include "Mesh.h"

class SphereSide {
	public:
		SphereSide(unsigned int& resolution, float& radius, glm::mat4& rotationMatrix, float& tile);
		std::vector <GLuint> indices;
};

#endif