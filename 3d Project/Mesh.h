#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include<string>

#include"VAO.h"
#include"EBO.h"
#include"Camera.h"
#include"Texture.h"
#include"Transform.h"

class Mesh
{
	public:
		std::vector <Vertex> vertices;
		std::vector <GLuint> indices;
		std::vector <Texture> textures;
		// Store VAO in public so it can be used in the Draw function
		VAO VAO;
		Transform transform;
		glm::mat4 matrix = glm::mat4(1.0f);

		//Shader& shader;
		//Camera& camera;


		// Initializes the mesh
		Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures);
		Mesh();


		// Draws the mesh
		void UpdateMesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures);
		void Draw(
			Shader& shader,
			Camera& camera
		);
};
#endif

