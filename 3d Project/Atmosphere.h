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
		float atmosphereScale = 1.2f;
		float densityFalloff = 5.0f;

		int inScatteringPoints = 10;
		int opticalDepthPoints = 8;
		float intensity = 1.0f;

		// Colo
		glm::vec3 wavelengths = glm::vec3(700.0f, 530.0f, 440.0f);
		float scatteringCoefficient = 400.0f;
		float scatteringStrength = 0.5f;

		glm::mat4 matrix;

		void GenerateBuffers();
		void Update(glm::vec3 position, Camera& camera, int& width, int& height, float& planetRadius, glm::vec3& lightPos);
		void imgui_updates();
};


#endif
