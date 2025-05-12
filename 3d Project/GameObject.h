#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Mesh.h"

struct Transform {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

class GameObject {
    Transform transform;
};

#endif