#ifndef NOISE_CLASS_H
#define NOISE_CLASS_H

#include "glm/glm.hpp"

class Noise {
	float noise(float x);
	float noise(glm::vec2 x);
	float noise(glm::vec3 x);
};

#endif

