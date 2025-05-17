#include "PlanetGenerator.h"

PlanetGenerator::PlanetGenerator(Sphere& sphere01, Mesh& mesh01) : sphere(sphere01), mesh(mesh01), atmosphere(atmosphere)
{
	// PlanetGenerator::mesh = mesh01;
	PlanetGenerator::resolution = sphere01.resolution;
	PlanetGenerator::radius = sphere01.radius;
	PlanetGenerator::tile = tile;
	PlanetGenerator::planetTex = mesh01.textures;
	// sphere01.~Sphere();

	atmosphere = Atmosphere();

	PlanetGenerator::UpdateMesh();
}

PlanetGenerator::PlanetGenerator(unsigned int resolution, float radius, float tile, Mesh& mesh01) : sphere(sphere), mesh(mesh01), atmosphere(atmosphere)
{
	// PlanetGenerator::mesh = mesh01;
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
	unsigned int numVerts = ((resolution + 1) * (resolution + 1)) * 6;

	if (sphere.resolution != PlanetGenerator::resolution || sphere.tile != tile) {
		// sphere.~Sphere();
		Sphere sphere(resolution, 1.0f, tile);
		PlanetGenerator::sphere = sphere;

	}

	computeVerts = sphere.computeVerts;
	vertices = sphere.vertices;
	indices = sphere.indices;


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
	newUp = normalize(camera.transform.position - transform.position);

	if (glm::distance(transform.position, camera.transform.position) < radius * atmosphere.atmosphereScale) { 
		camera.WorldUp = camera.LocalUp - (-newUp * delta * 10.0f);	
		camera.transform.parent = &transform;
	}
	else { 
		camera.WorldUp = camera.LocalUp - (-glm::vec3(0.0f, 1.0f, 0.0f) * delta * 10.0f); 
		camera.transform.parent = nullptr;
	}

}



void PlanetGenerator::Draw(Shader& shader, Camera& camera, glm::vec3& lightPos, glm::vec4 lightColor) {
	mesh.transform.position = transform.position;

	mesh.Draw(shader, camera);
	imgui_processing();

	atmosphere.Update(transform.position, camera, camera.width, camera.height, radius, lightPos);
}




void PlanetGenerator::imgui_processing() {


	if (ImGui::CollapsingHeader(name)) {
		if (ImGui::CollapsingHeader("Mesh")) {

			ImGui::SliderFloat3("Sphere Position", &transform.position.x, -500.0f, 500.0f);
			ImGui::SliderFloat3("Sphere Scale", &transform.scale.x, -2.0f, 2.0f);
			ImGui::SliderFloat3("Sphere Rotation", &transform.rotation.x, 0.0f, 360.0f);
		}
		if (ImGui::CollapsingHeader("Params")) {
			bool res = ImGui::SliderInt("Resolution", &resolution, 2, 512);
			bool rad = ImGui::SliderFloat("Radius", &radius, 0, 100);
			bool t = ImGui::SliderFloat("Tile", &tile, 0, 20.0f);
			ImGui::Text("Crater");
			bool nC = ImGui::SliderInt("Num Craters", &numCraters, 0, 100);
			bool cW = ImGui::SliderFloat("craterWidth", &craterWidth, -1.0f, 10.0f);
			bool cS = ImGui::SliderFloat("craterSteepness", &craterSteepness, -1.0f, 10.0f);
			bool cD = ImGui::SliderFloat("craterDepth", &craterDepth, -1.0f, 0.0f);
			bool rW = ImGui::SliderFloat("rimWidth", &rimWidth, -1.0f, 10.0f);
			bool rS = ImGui::SliderFloat("rimSteepness", &rimSteepness, 0.0f, 50.0f);
			bool rE = ImGui::SliderFloat("smoothingK", &smoothingK, 0.001f, 1.0f);
			if (res || rad || t || cW || cS || cD || rW || rS || rE || nC) {
				UpdateMesh();
			}
		}

		
		atmosphere.imgui_updates();

	}
}