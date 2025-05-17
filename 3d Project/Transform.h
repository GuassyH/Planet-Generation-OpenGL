#ifndef TRANSFORM_CLASS_H
#define TRANSFORM_CLASS_H
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

class Transform {
	public:

		Transform();

		Transform* parent = nullptr;

		// Global translations
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 rotation = glm::vec3(0.0f);
		glm::vec3 scale = glm::vec3(1.0f);

		// Local translations
		glm::vec3 localPosition = glm::vec3(0.0f);
		glm::vec3 localRotation = glm::vec3(0.0f);
		glm::vec3 localScale = glm::vec3(1.0f);
		
		// Directions
		glm::vec3 forward = glm::vec3(0.0f);
		glm::vec3 up = glm::vec3(0.0f);
		glm::vec3 right = glm::vec3(0.0f);

		void Update();

		~Transform();
};


#endif