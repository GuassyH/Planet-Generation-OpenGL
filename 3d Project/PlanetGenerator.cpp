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



void createComputeShader(unsigned int numVerts, float resolution, const char* ShaderName) {

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
	

	std::string computeShaderString = layoutStr + get_file_contents(ShaderName);
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


void runComputeShader(unsigned int numVerts, float resolution, GLuint& vertBuff, GLsizeiptr& vertBuffSize, std::vector<Vertex>& vertices, std::vector<ComputeVertex>& computeVerts) {


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


	vertices = sphere.vertices;
	indices = sphere.indices;


	unsigned int numVerts = vertices.size();

	// Only do the loop if the numverts has changed i could just read back, its faster
	if (baseComputeVerts.size() != numVerts) {
		std::vector<ComputeVertex>(numVerts).swap(baseComputeVerts);
		for (unsigned int i = 0; i < numVerts; i++)
		{
			baseComputeVerts[i].position = glm::vec4(vertices[i].position, 0.0f);
			baseComputeVerts[i].normal = glm::vec4(vertices[i].normal, 0.0f);
			baseComputeVerts[i].color = glm::vec4(vertices[i].color, 0.0f);
			baseComputeVerts[i].texUV = glm::vec4(vertices[i].texUV, 0.0f, 0.0f);
		}
		std::cout << "Creating new Comp Verts" << std::endl;
	}
	
	std::vector<ComputeVertex> computeVerts = baseComputeVerts;

	// RUN VERTEX HEIGHT CALCULATION
	createComputeShader(numVerts, resolution, "ComputePlanet.txt");

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
	createComputeShader(numVerts, resolution, "ComputeNormals.txt");

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


	std::vector<ComputeVertex>().swap(computeVerts);


	mesh.UpdateMesh(vertices, indices, planetTex);
}




void PlanetGenerator::Draw(Shader& shader, Camera& camera) {
	mesh.Draw(shader, camera);
	
	
	atmosphere.Update(mesh.position);
}