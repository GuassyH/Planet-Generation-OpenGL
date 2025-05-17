#include "Noise.h"


//	<https://www.shadertoy.com/view/4dS3Wd>
//	By Morgan McGuire @morgan3d, http://graphicscodex.com
//
float hash(float n) { return glm::fract(sin(n) * 1e4); }
float hash(glm::vec2 p) { return glm::fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float Noise::noise(float x) {
	float i = glm::floor(x);
	float f = glm::fract(x);
	float u = f * f * (3.0 - 2.0 * f);
	return glm::mix(hash(i), hash(i + 1.0), u);
}

float Noise::noise(glm::vec2 x) {
	glm::vec2 i = glm::floor(x);
	glm::vec2 f = glm::fract(x);

	// Four corners in 2D of a tile
	float a = hash(i);
	float b = hash(i + glm::vec2(1.0, 0.0));
	float c = hash(i + glm::vec2(0.0, 1.0));
	float d = hash(i + glm::vec2(1.0, 1.0));


	glm::vec2 u = f * f * (3.0f - 2.0f * f);
	return glm::mix(a, b, u.x) + (c - a) * u.y * (1.0f - u.x) + (d - b) * u.x * u.y;
}

// This one has non-ideal tiling properties that I'm still tuning
float Noise::noise(glm::vec3 x) {
	const glm::vec3 step = glm::vec3(110, 241, 171);

	glm::vec3 i = floor(x);
	glm::vec3 f = fract(x);

	// For performance, compute the base input to a 1D hash from the integer part of the argument and the 
	// incremental change to the 1D based on the 3D -> 1D wrapping
	float n = glm::dot(i, step);

	glm::vec3 u = f * f * (3.0f - 2.0f * f);
	return glm::mix(glm::mix(glm::mix(hash(n + glm::dot(step, glm::vec3(0, 0, 0))), hash(n + glm::dot(step, glm::vec3(1, 0, 0))), u.x),
		glm::mix(hash(n + glm::dot(step, glm::vec3(0, 1, 0))), hash(n + glm::dot(step, glm::vec3(1, 1, 0))), u.x), u.y),
		glm::mix(glm::mix(hash(n + glm::dot(step, glm::vec3(0, 0, 1))), hash(n + glm::dot(step, glm::vec3(1, 0, 1))), u.x),
			glm::mix(hash(n + glm::dot(step, glm::vec3(0, 1, 1))), hash(n + glm::dot(step, glm::vec3(1, 1, 1))), u.x), u.y), u.z);
}