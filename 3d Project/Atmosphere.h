#ifndef ATMOSPHERE_CLASS_H
#define ATMOSPHERE_CLASS_H

#include "Mesh.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

		int width;
		int height;

		// Atmoshpere Settings
		float atmosphereRadius = 60.0f;
		float densityFallOff = 5.0f;

		int numInScatteringPoints = 8;
		int numOpticalDepthPoints = 3;
		float intensity = 1.0f;

		// Colo
		glm::vec3 wavelengths = glm::vec3(700.0f, 550.0f, 440.0f);
		float scatteringStrength = 0.5f;

		glm::mat4 matrix;

		void GenerateBuffers();
		void Update(glm::vec3 position, Camera& camera, int& width, int& height, float& planetRadius, glm::vec3& lightPos);
		void imgui_updates();
};


#endif
