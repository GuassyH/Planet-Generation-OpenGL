#include "Transform.h"

Transform::Transform(){
	// std::cout << "Init Transform" << std::endl;
}

void Transform::Update() {
	localPosition = parent == nullptr ? glm::vec3(0.0f) : parent->position - position;
	localRotation = parent == nullptr ? glm::vec3(0.0f) : parent->rotation - rotation;
	localScale = parent == nullptr ? glm::vec3(1.0f) : (parent->scale + glm::vec3(1.0f)) - scale;

	forward = glm::vec3(0.0f);
	right = glm::normalize(glm::cross(forward, up));
	up = glm::normalize(glm::cross(right, forward));
}

Transform::~Transform() {
	// std::cout << "Init Transform" << std::endl;
}