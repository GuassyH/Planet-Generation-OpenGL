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

Atmosphere::Atmosphere() : atmosphereShader(atmosphereShader), AtmosphereName(AtmosphereName), rectVAO(rectVAO), rectVBO(rectVBO), FBO(FBO), RBO(RBO), frameBufferTexture(frameBufferTexture) {
	// Create new Atmosphere Shader

	Atmosphere::atmosphereShader = Shader("Atmosphere.vert", "Atmosphere.frag");

	
	AtmosphereName = "Planet Atmosphere";
	std::cout << AtmosphereName << std::endl;


	atmosphereShader.Activate();
	glUniform1i(glGetUniformLocation(atmosphereShader.ID, "screenTexture"), 0);


	int height;
	int width;
	
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


	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	Atmosphere::RBO = RBO;


	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Atmosphere Framebuffer error: " << fboStatus << std::endl;
	}

}



void Atmosphere::Update(glm::vec3 position) {
	
	// Set all variables
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Activate shader

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	atmosphereShader.Activate();
	glm::mat4 pos = glm::translate(glm::mat4(1.0f), position);
	glUniformMatrix4fv(glGetUniformLocation(atmosphereShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(matrix));
	glUniformMatrix4fv(glGetUniformLocation(atmosphereShader.ID, "position"), 1, GL_FALSE, glm::value_ptr(pos));

	glBindVertexArray(rectVAO);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}