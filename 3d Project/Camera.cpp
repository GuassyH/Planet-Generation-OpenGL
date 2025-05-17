#include "Camera.h"


Camera::Camera(int width, int height, glm::vec3 position) {
	Camera::width = width;
	Camera::height = height;
}



void Camera::Matrix(Shader& shader, const char* uniform) {
	// Set "camMatrix" or other uniform equal to the projection * view (cameraMatrix)
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::UpdateMatrix(float FOVdeg, float nearPlane, float farPlane, int windowWidth, int windowHeight) {
	// Initialise the matrices
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);


	// set the width to the windowWidth etc, helps for setting glm::perspective if window size changes
	Camera::width = windowWidth;
	Camera::height = windowHeight;

	// view = lookAt a vec3, projection = perspective I give it
	view = glm::lookAt(transform.position, transform.position + Orientation, WorldUp);
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / (float)height, nearPlane, farPlane);



	LocalForward = glm::normalize(Orientation);
	Right = glm::normalize(glm::cross(LocalForward, WorldUp));
	LocalRight = glm::normalize(glm::cross(Orientation, LocalUp));
	LocalUp = glm::normalize(glm::cross(Right, LocalForward));

	Camera::FOVdeg = FOVdeg;
	Camera::farPlane = farPlane;
	Camera::nearPlane = nearPlane;

	cameraMatrix = projection * view;
}




void Camera::Inputs(GLFWwindow* window) {

	// X & Z
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { transform.position += speed * LocalForward; }
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { transform.position += speed * -LocalRight; }
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { transform.position += speed * -LocalForward; }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { transform.position += speed * LocalRight; }
	// Y
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) { transform.position += speed * LocalUp; }
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { transform.position += speed * -LocalUp; }
	// Speed control
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) { speed = 0.5f; }
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) { speed = 0.05f; }
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE){
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE){
			speed = 0.1f; 
		}	
	}

	// Mouse movement
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick) {
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		// Check if the angle to the new rotation is NOT less than 5deg to being 90deg
		// Need to make orientation be the forward of the body, not of the camera
		glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, WorldUp)));
		if (!((glm::angle(newOrientation, WorldUp) <= glm::radians(5.0f)) || (glm::angle(newOrientation, -WorldUp) <= glm::radians(5.0f)))) {
			Orientation = newOrientation;
		}
		Orientation = glm::rotate(Orientation, glm::radians(-rotY), WorldUp);

		glfwSetCursorPos(window, (width / 2), (height / 2));

	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
}