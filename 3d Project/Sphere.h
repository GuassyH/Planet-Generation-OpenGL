#ifndef SPHERE_CLASS_H
#define SPHERE_CLASS_H

#include "Mesh.h"

class Sphere {
	public:

		float radius;
		unsigned int resolution;
		float tile;

		Sphere(unsigned int resolution, float radius, float tile);
		void runComputeSphere(unsigned int& resolution, float& radius, float& tile, unsigned int& numVerts, std::vector<Vertex>& verts);
		~Sphere();

		std::vector <Vertex> vertices;
		std::vector <ComputeVertex> computeVerts;
		std::vector <GLuint> indices;
};

#endif