#include "Atmosphere.h"

float rectangleVertices[] = {
	
	//  COORDS   //  TEX UV
	1.0f, -1.0f,	1.0f, 0.0f,
	-1.0f, -1.0f,	0.0f, 0.0f,
	-1.0f, 1.0f,		0.0f, 1.0f,

	1.0f, 1.0f,		1.0f, 1.0f,
	1.0f, -1.0f,	1.0f, 0.0f,
	-1.0f, 1.0f,	0.0f, 1.0f,

};


void Atmosphere::GenerateBuffers() {

	atmosphereShader.Activate();
	glUniform1i(glGetUniformLocation(atmosphereShader.ID, "screenTexture"), 0);


	glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);


	unsigned int rectVAO;
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	Atmosphere::rectVAO = rectVAO;


	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	Atmosphere::FBO = FBO;

	unsigned int frameBufferTexture;
	glGenTextures(1, &frameBufferTexture);
	glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);
	Atmosphere::frameBufferTexture = frameBufferTexture;

	unsigned int depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	Atmosphere::depthTexture = depthTexture;


	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Atmosphere Framebuffer error: " << fboStatus << std::endl;
	}

}

Atmosphere::Atmosphere() : atmosphereShader(atmosphereShader), name(name), rectVAO(rectVAO), rectVBO(rectVBO), FBO(FBO), depthTexture(depthTexture), frameBufferTexture(frameBufferTexture) {
	// Create new Atmosphere Shader

	Atmosphere::atmosphereShader = Shader("Atmosphere.vert", "Atmosphere.frag");

	name = "Planet Atmosphere";

	GenerateBuffers();

}



void Atmosphere::Update(glm::vec3 position, Camera& camera, int& width, int& height, float& planetRadius, glm::vec3& lightPos) {
	


	if (Atmosphere::width != width || Atmosphere::height != height) {
		GenerateBuffers();
	}


	// Set all variables
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glUniform1i(glGetUniformLocation(atmosphereShader.ID, "depthTexture"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// Activate shader
	atmosphereShader.Activate();


	// ================ //

	#pragma region Assign Variables

	float scatterR = glm::pow(scatteringCoefficient / wavelengths.x, 4) * scatteringStrength;
	float scatterG = glm::pow(scatteringCoefficient / wavelengths.y, 4) * scatteringStrength;
	float scatterB = glm::pow(scatteringCoefficient / wavelengths.z, 4) * scatteringStrength;



	glUniform3f(glGetUniformLocation(atmosphereShader.ID, "atmosphereCentre"), position.x, position.y, position.z);
	glUniform2f(glGetUniformLocation(atmosphereShader.ID, "screenResolution"), float(width), float(height));
	glUniform3f(glGetUniformLocation(atmosphereShader.ID, "sunPos"), lightPos.x, lightPos.y, lightPos.z);

	glUniform1f(glGetUniformLocation(atmosphereShader.ID, "FOVdeg"), camera.FOVdeg);

	glUniform3f(glGetUniformLocation(atmosphereShader.ID, "camPos"), camera.transform.position.x, camera.transform.position.y, camera.transform.position.z);
	glUniform3f(glGetUniformLocation(atmosphereShader.ID, "camUp"), camera.LocalUp.x, camera.LocalUp.y, camera.LocalUp.z);
	glUniform3f(glGetUniformLocation(atmosphereShader.ID, "camForward"), camera.LocalForward.x, camera.LocalForward.y, camera.LocalForward.z);
	glUniform3f(glGetUniformLocation(atmosphereShader.ID, "camRight"), camera.LocalRight.x, camera.LocalRight.y, camera.LocalRight.z);
	glUniform1f(glGetUniformLocation(atmosphereShader.ID, "camFarPlane"), camera.farPlane);
	glUniform1f(glGetUniformLocation(atmosphereShader.ID, "camNearPlane"), camera.nearPlane);

	// ATMOSPHERE SETTINGS
	glUniform1f(glGetUniformLocation(atmosphereShader.ID, "atmosphereScale"), atmosphereScale);
	glUniform1f(glGetUniformLocation(atmosphereShader.ID, "planetRadius"), planetRadius);
	glUniform1f(glGetUniformLocation(atmosphereShader.ID, "densityFalloff"), densityFalloff);

	glUniform1i(glGetUniformLocation(atmosphereShader.ID, "numInScatteringPoints"), inScatteringPoints);
	glUniform1i(glGetUniformLocation(atmosphereShader.ID, "numOpticalDepthPoints"), opticalDepthPoints);
	glUniform1f(glGetUniformLocation(atmosphereShader.ID, "intensity"), intensity);
	// Color
	glUniform3f(glGetUniformLocation(atmosphereShader.ID, "scatteringCoefficients"), scatterR, scatterG, scatterB);

	#pragma endregion

	// ================ //



	camera.Matrix(atmosphereShader, "camMatrix");
	

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);  // <--- Prevent writing to depth buffer

	glBindVertexArray(rectVAO);
	glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Re-enable depth writes and depth test after you're done
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);


}


void Atmosphere::imgui_updates() {

	if (ImGui::CollapsingHeader("Atmosphere")) {
		ImGui::SliderFloat("Atmosphere Scale", &atmosphereScale, 1.0f, 5.0f);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::SliderFloat("Density Falloff", &densityFalloff, -2.0f, 20.0f);
		ImGui::SliderInt("numInScatteringPoints", &inScatteringPoints, 1, 20);
		ImGui::SliderInt("numOpticalDepthPoints", &opticalDepthPoints, 1, 20);
		ImGui::SliderFloat("intensity", &intensity, 0.0f, 2.0f);
		ImGui::SliderFloat3("wavelengths", &wavelengths.x, 0.0f, 1000.0f);
		ImGui::SliderFloat("scatteringCoefficient", &scatteringCoefficient, 0.0f, 700.0f);
		ImGui::SliderFloat("scatteringStrength", &scatteringStrength, 0.0f, 10.0f);
	}
}