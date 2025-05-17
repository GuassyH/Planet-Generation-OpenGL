#include "Mesh.h"
#include "Cube.h"
#include "Sphere.h"
#include "Model.h"
#include "PlanetGenerator.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


int windowWidth = 1280;
int windowHeight = 720;


float refreshRate;
int monitorWidth;
int monitorHeight;

void imgui_processing(Mesh& light);
void frame_buffer_size_callback(GLFWwindow* window, int width, int height);
void process_inputs(GLFWwindow* window, GLFWmonitor* monitor, Camera& camera, float& delta);

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
	Mesh planetAMesh;
	planetAMesh.textures = planetTex;
	PlanetGenerator planetA(128, 50.0f, 1.0f, planetAMesh);
	planetA.name = "Planet A";




	//		-	MOVING AND ASSIGNING MODELS		-		//
	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 0.8f, 1.0f);

	planetA.transform.position = glm::vec3(0.0f, 0.0f, -500.0f);
	light.transform.position = glm::vec3(0.0f, 0.0f, 500.0f);
	light.transform.scale = glm::vec3(100.0f);



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

	glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

	while (!glfwWindowShouldClose(window)) {

		
		if (!io.WantCaptureMouse){	process_inputs(window, monitor, camera, deltaTime);	}
		glBindFramebuffer(GL_FRAMEBUFFER, planetA.atmosphere.FBO);
		
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth, windowHeight);

		imgui_processing(light);


		crntTime = static_cast<float>(glfwGetTime());
		deltaTime = (crntTime - prevTime);
		prevTime = crntTime;


		camera.UpdateMatrix(75.0f, 0.1f, 2000.0f, windowWidth, windowHeight);


		light.Draw(lightShader, camera);
		planetA.Draw(planetShader, camera, light.transform.position, lightColor);
		planetA.CameraReOrient(camera, deltaTime);



		lightShader.Activate();
		glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		planetShader.Activate();
		glUniform4f(glGetUniformLocation(planetShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
		glUniform3f(glGetUniformLocation(planetShader.ID, "lightPos"), light.transform.position.x, light.transform.position.y, light.transform.position.z);
		glUniform3f(glGetUniformLocation(planetShader.ID, "planetPos"), planetA.transform.position.x, planetA.transform.position.y, planetA.transform.position.z);



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
	glDeleteProgram(planetA.computePlanetProgram);

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}



void imgui_processing(Mesh& light) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	ImGui::Begin("Debug Window");
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
	if (ImGui::CollapsingHeader("Main Light")) {
		ImGui::Text("Light");
		ImGui::SliderFloat3("Light Position", &light.transform.position.x, -2000.0f, 1000.0f);
		ImGui::SliderFloat3("Light Scale", &light.transform.scale.x, -20.0f, 20.0f);
	}

}


void frame_buffer_size_callback(GLFWwindow* window, int width, int height) {
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);
	std::cout << "window size is " << width << " x " << height << std::endl;
}




void process_inputs(GLFWwindow* window, GLFWmonitor* monitor, Camera& camera, float& delta) {
	
	camera.Inputs(window, delta);
	
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