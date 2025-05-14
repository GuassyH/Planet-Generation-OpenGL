#include "Mesh.h"
#include "Cube.h"
#include "Sphere.h"
#include "Model.h"
#include "GameObject.h"
#include "PlanetGenerator.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


int windowWidth = 1280;
int windowHeight = 720;


float refreshRate;
int monitorWidth;
int monitorHeight;

void imgui_processing(Mesh& sphere, Mesh& light, PlanetGenerator& planetA);
void frame_buffer_size_callback(GLFWwindow* window, int width, int height);
void process_inputs(GLFWwindow* window, GLFWmonitor* monitor, Camera& camera);

std::vector<Mesh> meshes;







int main(void) {

#pragma region Initialising GLFW


	if (!glfwInit()) {
		return -1;
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor* monitor;
	GLFWwindow* window;

	monitor = glfwGetPrimaryMonitor();
	window = glfwCreateWindow(windowWidth, windowHeight, "3d Project - Scene View", NULL, NULL);
	
	if (!window) {
		std::cout << "Failed to create Window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	gladLoadGL();
	glViewport(0, 0, windowWidth, windowHeight);

	
	//		-	CREATING Verts	-		//


	// Texture data
	Texture textures[]
	{
		Texture("planks.png", "diffuse", 0),
		Texture("planksSpec.png", "specular", 1)
	};
	Texture planetTextures[]
	{
		Texture("gravel.png", "diffuse", 0),
		Texture("planksSpec.png", "specular", 1)
	};


	glfwSetFramebufferSizeCallback(window, frame_buffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Get the monitors width and height
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	monitorWidth = mode->width;
	monitorHeight = mode->height;

	std::cout << monitorWidth << " x " << monitorHeight << std::endl;


#pragma endregion



	Camera camera(windowWidth, windowHeight, glm::vec3(0.0f, 0.5f, 75.0f));


	//		- CREATING OBJS -	


	Shader lightShader("light.vert", "light.frag");
	Shader planetShader("planet.vert", "planet.frag");


	std::vector <Texture> planetTex(planetTextures, planetTextures + sizeof(planetTextures) / sizeof(Texture));
	std::vector <Texture> lightTex(textures, textures + sizeof(textures) / sizeof(Texture));


	// Create Cube mesh
	Sphere i_lightSphere(5, 0.1f, 1.0f);
	Mesh light(i_lightSphere.vertices, i_lightSphere.indices, lightTex);

	// Create Sphere mesh
	Mesh sphereMesh;
	sphereMesh.textures = planetTex;
	PlanetGenerator planetA(50, 50.0f, 1.0f, sphereMesh);

	


	//		-	MOVING AND ASSIGNING MODELS		-		//
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 0.8f, 1.0f);

	sphereMesh.position = glm::vec3(0.0f, 0.0f, 0.0f);
	light.position = glm::vec3(0.0f, 0.0f, 100.0f);
	light.scale = glm::vec3(10.0f);

	lightShader.Activate();
	glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	planetShader.Activate();
	glUniform4f(glGetUniformLocation(planetShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(planetShader.ID, "lightPos"), light.position.x, light.position.y, light.position.z);




	// IMGUI Stuff
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");




	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	// glFrontFace(GL_CW);

	float prevTime = static_cast<float>(glfwGetTime());
	float crntTime = 0.0f;
	float deltaTime = 0.0f;

	float gravity = -9.82f;
	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

	while (!glfwWindowShouldClose(window)) {

		if (!io.WantCaptureMouse){	process_inputs(window, monitor, camera);	}

		glBindFramebuffer(GL_FRAMEBUFFER, planetA.atmosphere.FBO);

		glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, windowWidth, windowHeight);

		imgui_processing(sphereMesh, light, planetA);




		crntTime = static_cast<float>(glfwGetTime());
		deltaTime = (crntTime - prevTime);
		prevTime = crntTime;

		velocity += glm::vec3(0.0f, gravity * deltaTime, 0.0f);

		if (sphereMesh.position.y < -10.0f) {
			velocity = glm::vec3(0.0f, 9.81f, 0.0f);
		}
		// sphereMesh.position += velocity * deltaTime;

		camera.UpdateMatrix(60.0f, 0.1f, 500.0f, windowWidth, windowHeight);



		light.Draw(lightShader, camera);
		planetA.Draw(planetShader, camera);


		glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform4f(glGetUniformLocation(planetShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(glGetUniformLocation(planetShader.ID, "lightPos"), light.position.x, light.position.y, light.position.z);




		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Delete shader programs
	lightShader.Delete();
	planetShader.Delete();
	planetA.atmosphere.atmosphereShader.Delete();

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}



void imgui_processing(Mesh& sphere, Mesh& light, PlanetGenerator& planetA) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();


	ImGui::Begin("Debug Window");
	if (ImGui::CollapsingHeader("Transforms")) {
		ImGui::Text("Sphere");
		ImGui::SliderFloat3("Sphere Position", &sphere.position.x, -10.0f, 10.0f);
		ImGui::SliderFloat3("Sphere Scale", &sphere.scale.x, -2.0f, 2.0f);
		ImGui::SliderFloat3("Sphere Rotation", &sphere.rotation.x, 0.0f, 360.0f);
		ImGui::Text("Light");
		ImGui::SliderFloat3("Light Position", &light.position.x, -100.0f, 100.0f);
		ImGui::SliderFloat3("Light Scale", &light.scale.x, -2.0f, 2.0f);
	}
	if (ImGui::CollapsingHeader("Planet Generation")) {

		bool res = ImGui::SliderInt("Resolution", &planetA.resolution, 2, 512);
		bool rad = ImGui::SliderFloat("Radius", &planetA.radius, 0, 100);
		bool tile = ImGui::SliderFloat("Tile", &planetA.tile, 0, 20.0f);
		ImGui::Text("Crater");
		bool nC = ImGui::SliderInt("Num Craters", &planetA.numCraters, 0, 100);
		bool cW = ImGui::SliderFloat("craterWidth", &planetA.craterWidth, -1.0f, 10.0f);
		bool cS = ImGui::SliderFloat("craterSteepness", &planetA.craterSteepness, -1.0f, 10.0f);
		bool cD = ImGui::SliderFloat("craterDepth", &planetA.craterDepth, -1.0f, 0.0f);
		bool rW = ImGui::SliderFloat("rimWidth", &planetA.rimWidth, -1.0f, 10.0f);
		bool rS = ImGui::SliderFloat("rimSteepness", &planetA.rimSteepness, 0.0f, 50.0f);
		bool rE = ImGui::SliderFloat("smoothingK", &planetA.smoothingK, 0.001f, 1.0f);


		if (res || rad || tile || cW || cS || cD || rW || rS || rE || nC) {
			planetA.UpdateMesh();
		}
		
	}
}


void frame_buffer_size_callback(GLFWwindow* window, int width, int height) {
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);
	std::cout << "window size is " << width << " x " << height << std::endl;
}




void process_inputs(GLFWwindow* window, GLFWmonitor* monitor, Camera& camera) {
	
	camera.Inputs(window);
	
	// For setting window size
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
		glfwSetWindowSize(window, 1080, 720);
		glfwSetWindowPos(window, (monitorWidth - windowWidth) / 2, (monitorHeight - windowHeight) / 2);
		std::cout << "set window size 1280 x 720" << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
		glfwSetWindowSize(window, 1920, 1080);
		glfwSetWindowPos(window, (monitorWidth - windowWidth) / 2, (monitorHeight - windowHeight) / 2);
		std::cout << "set window size 1920 x 1080" << std::endl;
	}	
	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
		glfwSetWindowSize(window, 2560, 1440);
		glfwSetWindowPos(window, (monitorWidth - windowWidth) / 2, (monitorHeight - windowHeight) / 2);
		std::cout << "set window size 2560 x 1440" << std::endl;
	}	


	if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}


	// Terminating the process
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}


}