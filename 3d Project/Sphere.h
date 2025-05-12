#ifndef SPHERE_CLASS_H
#define SPHERE_CLASS_H

#include "Mesh.h"

class Sphere {
	public:

		float radius;
		unsigned int resolution;
		float tile;

		Sphere(unsigned int resolution, float radius, float tile);
		~Sphere();

		std::vector <Vertex> vertices;
		std::vector <GLuint> indices;
};

#endif