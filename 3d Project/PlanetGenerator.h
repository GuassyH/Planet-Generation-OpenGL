#ifndef PLANET_GENERATOR_H
#define PLANET_GENERATOR_H

#include "Mesh.h"
#include "Sphere.h"
#include <glm/gtc/random.hpp>
#include "Atmosphere.h"

class PlanetGenerator {
public:
	float frequency = 10.0f;
	float amplitude = 10.0f;

	int resolution = 50;
	float radius = 2.0f;
	float tile = 1.0f;
	// Crater
	int numCraters = 0;
	float craterWidth = 2.9f;
	float craterSteepness = 1.2f;
	float craterDepth = -1.0f;
	// Crater Rim
	float rimWidth = 1.0f;
	float rimSteepness = 10.0f;
	float smoothingK = 0.1f;

	float maxHeight = 0.0f;

	Mesh& mesh;
	Sphere sphere;

	GLuint computePlanetProgram;

	Atmosphere atmosphere;

	std::vector<Vertex> vertices;
	std::vector<ComputeVertex> baseComputeVerts;

	std::vector<GLuint> indices;
	std::vector<Texture> planetTex;
	std::vector<glm::vec4> craterPositions;

	PlanetGenerator(Sphere& sphere, Mesh& mesh);
	PlanetGenerator(unsigned int resolution, float radius, float tile, Mesh& mesh01);
	void UpdateMesh();
	void Draw(Shader& shader, Camera& camera);
};

#endif