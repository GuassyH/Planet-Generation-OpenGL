#ifndef ATMOSPHERE_CLASS_H
#define ATMOSPHERE_CLASS_H

#include "Mesh.h"


class Atmosphere {

	public:
		const char* AtmosphereName;

		Atmosphere();
		Shader atmosphereShader;

		unsigned int rectVAO;
		unsigned int rectVBO;

		unsigned int FBO;
		unsigned int frameBufferTexture;
		unsigned int RBO;

		glm::mat4 matrix;

		void Update(glm::vec3 position);
};


#endif
