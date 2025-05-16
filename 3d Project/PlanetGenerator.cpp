#include "PlanetGenerator.h"

PlanetGenerator::PlanetGenerator(Sphere& sphere01, Mesh& mesh01) : sphere(sphere01), mesh(mesh01), atmosphere(atmosphere)
{
	PlanetGenerator::mesh = mesh01;
	PlanetGenerator::resolution = sphere01.resolution;
	PlanetGenerator::radius = sphere01.radius;
	PlanetGenerator::tile = tile;
	PlanetGenerator::planetTex = mesh01.textures;
	sphere01.~Sphere();

	atmosphere = Atmosphere();

	PlanetGenerator::UpdateMesh();
}

PlanetGenerator::PlanetGenerator(unsigned int resolution, float radius, float tile, Mesh& mesh01) : sphere(sphere), mesh(mesh01), atmosphere(atmosphere)
{
	PlanetGenerator::mesh = mesh01;
	PlanetGenerator::resolution = resolution;
	PlanetGenerator::radius = radius;
	PlanetGenerator::tile = tile;
	PlanetGenerator::planetTex = mesh01.textures;

	atmosphere = Atmosphere();

	PlanetGenerator::UpdateMesh();
}



GLuint computeProgram;
GLuint computeShader;

GLuint workGroupSize;



void createComputeShader(unsigned int& numVerts, int& resolution, const char* ShaderName) {

	int success;
	char InfoLog[512];

	workGroupSize = 1;
	for (unsigned int i = 12; i > 0; i--) {
		if ((static_cast<unsigned int>(resolution) + 1) % i == 0) {
			workGroupSize = i;
			break;
		}
	}

	unsigned int layoutGroupX = workGroupSize;
	unsigned int layoutGroupY = workGroupSize;
	unsigned int layoutGroupZ = 6;

	std::string layoutStr = 
	"#version 460 core \n layout(local_size_x = " + std::to_string(layoutGroupX) + ", local_size_y = " + std::to_string(layoutGroupY) + ", local_size_z = " + std::to_string(layoutGroupZ) + ") in; ";
	
	// comment out #version 460 core in the shader file which is used to stop syntax errors
	std::string computeShaderString = layoutStr + "//" + get_file_contents(ShaderName);
	const char* computeShaderSource = computeShaderString.c_str();

	computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &computeShaderSource, NULL);
	glCompileShader(computeShader);
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(computeShader, 512, 0, InfoLog);
		std::cout << "Failed to compile planet compute shader - ERR: " << InfoLog << std::endl;
	}

	computeProgram = glCreateProgram();
	glAttachShader(computeProgram, computeShader);
	glLinkProgram(computeProgram);
	glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(computeProgram, 512, 0, InfoLog);
		std::cout << "Failed to link planet compute program - ERR: " << InfoLog << std::endl;
	}

	glUseProgram(computeProgram);


}

void runComputeShader(unsigned int& numVerts, int& resolution, GLuint& vertBuff, GLsizeiptr& vertBuffSize, std::vector<Vertex>& vertices, std::vector<ComputeVertex>& computeVerts) {


	glDispatchCompute((resolution + 1) / workGroupSize, (resolution + 1) / workGroupSize, 6);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertBuff);

	// Map buffer for reading
	ComputeVertex* ptr = (ComputeVertex*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, vertBuffSize, GL_MAP_READ_BIT);
	

	if (ptr == nullptr) {
		std::cerr << "Failed to map planet buffer for reading!" << std::endl;
	}
	else {
		for (unsigned int i = 0; i < numVerts; ++i) {
			computeVerts[i] = ptr[i]; // copy from GPU buffer to CPU vector

			vertices[i].position = glm::vec3(computeVerts[i].position);
			vertices[i].normal = glm::vec3(computeVerts[i].normal);
			vertices[i].color = glm::vec3(computeVerts[i].color);
			vertices[i].texUV = glm::vec2(computeVerts[i].texUV);

		}
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER); // VERY IMPORTANT

	glDeleteShader(computeShader);
	glDeleteProgram(computeProgram);
}


void PlanetGenerator::UpdateMesh() {

	// Create new sphere if needed
	if (sphere.resolution != PlanetGenerator::resolution || sphere.tile != tile) {
		// sphere.~Sphere();
		Sphere sphere(resolution, 1.0f, tile);
		PlanetGenerator::sphere = sphere;

	}


	int craterPosSize = craterPositions.size();
	if (numCraters != craterPosSize) {
		if (numCraters > craterPosSize) {
			for (unsigned int i = 0; i < numCraters - craterPosSize; i++)
			{
				glm::vec4 newPos = glm::vec4(glm::sphericalRand(1.0f), 1.0f);
				craterPositions.push_back(newPos);
			}
		}
		else {
			for (unsigned int i = 0; i < craterPosSize - numCraters; i++)
			{
				craterPositions.pop_back();
			}
		}
	}


	computeVerts = sphere.computeVerts;
	vertices = sphere.vertices;
	indices = sphere.indices;


	unsigned int numVerts = vertices.size();



	// RUN VERTEX HEIGHT CALCULATION
	createComputeShader(numVerts, resolution, "ComputePlanet.comp");

	// assign buffer
	GLuint vertBuff;
	GLsizeiptr vertBuffSize = sizeof(ComputeVertex) * numVerts;

	GLuint craterBuff;
	GLsizeiptr craterBuffSize = sizeof(glm::vec4) * numCraters;

	glGenBuffers(1, &vertBuff);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertBuff);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertBuffSize, computeVerts.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertBuff); // THIS IS ESSENTIAL

	// NOT WORKING ?!!?
	glCreateBuffers(1, &craterBuff);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, craterBuff);
	glBufferData(GL_SHADER_STORAGE_BUFFER, craterBuffSize, craterPositions.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, craterBuff); // THIS IS ESSENTIAL

	// Basics
	glUniform1i(glGetUniformLocation(computeProgram, "numVerts"), numVerts);
	glUniform1f(glGetUniformLocation(computeProgram, "radius"), radius);
	glUniform1i(glGetUniformLocation(computeProgram, "resolution"), resolution);

	// Crater
	glUniform1i(glGetUniformLocation(computeProgram, "numCraters"), numCraters);
	glUniform1f(glGetUniformLocation(computeProgram, "craterWidth"), craterWidth);
	glUniform1f(glGetUniformLocation(computeProgram, "craterSteepness"), craterSteepness);
	glUniform1f(glGetUniformLocation(computeProgram, "craterDepth"), craterDepth);
	// Rim
	glUniform1f(glGetUniformLocation(computeProgram, "rimSteepness"), rimSteepness);
	glUniform1f(glGetUniformLocation(computeProgram, "rimWidth"), rimWidth);
	glUniform1f(glGetUniformLocation(computeProgram, "smoothingK"), smoothingK);


	runComputeShader(numVerts, resolution, vertBuff, vertBuffSize, vertices, computeVerts);


	glDeleteBuffers(1, &vertBuff);
	glDeleteBuffers(1, &craterBuff);
	
	

	// RUN NORMAL CALCULATION
	createComputeShader(numVerts, resolution, "ComputeNormals.comp");

	vertBuff;
	vertBuffSize = sizeof(ComputeVertex) * numVerts;

	// assign buffer
	glGenBuffers(1, &vertBuff);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertBuff);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertBuffSize, computeVerts.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertBuff); 


	glUniform1i(glGetUniformLocation(computeProgram, "numVerts"), numVerts);
	glUniform1i(glGetUniformLocation(computeProgram, "resolution"), resolution);

	runComputeShader(numVerts, resolution, vertBuff, vertBuffSize, vertices, computeVerts);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Delete all buffers
	glDeleteBuffers(1, &vertBuff);
	glDeleteBuffers(1, &craterBuff);



	mesh.UpdateMesh(vertices, indices, planetTex);

	std::vector<ComputeVertex>().swap(computeVerts);
	std::vector<Vertex>().swap(vertices);
	std::vector<GLuint>().swap(indices);
}

void PlanetGenerator::CameraReOrient(Camera& camera, float& delta) {
	glm::vec3 newUp;
	newUp = camera.Position - mesh.position;

	if (glm::distance(mesh.position, camera.Position) < radius * atmosphere.atmosphereScale) { camera.WorldUp = camera.WorldUp - (-newUp * delta * 0.25f); }
	else { camera.WorldUp = glm::vec3(0.0f, 1.0f, 0.0f) - (camera.WorldUp * delta * 0.25f); }
}


void PlanetGenerator::Draw(Shader& shader, Camera& camera, int& width, int& height, glm::vec3& lightPos, glm::vec4 lightColor) {
	mesh.Draw(shader, camera);
	
	atmosphere.Update(mesh.position, camera, width, height, radius, lightPos);
}