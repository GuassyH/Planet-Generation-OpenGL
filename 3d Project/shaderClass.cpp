#include "shaderClass.h"




Shader::Shader(const char* vertexFile, const char* fragmentFile) {


	//		-	CREATE SHADERS	-		//


	// get the context of the file and copy to a string
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

	// create a character array of the code from string to char
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	// Success log
	int success;
	char InfoLog[512];

	// Create vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	
	// Set vertexShader source to the vertex source (char array we created earlier)
	glShaderSource(vertexShader, 1, &vertexSource, 0);
	glCompileShader(vertexShader);
	
	// Debug
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, 0, InfoLog);
		std::cout << "Failed to compile vert shader - ERR: " << InfoLog << std::endl;
	}

	// Create fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	
	// Set fragment shader source to the fragment source
	glShaderSource(fragmentShader, 1, &fragmentSource, 0);
	glCompileShader(fragmentShader);

	// Debug
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, 0, InfoLog);
		std::cout << "Failed to compile vert shader - ERR: " << InfoLog << std::endl;
	}


	//		-	CREATE PROGRAM	-		//


	// This ID = the ID of the new Program
	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);

	// Links the shaders in it so they can communicate i THINK and creates and executable with both files
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, 0, InfoLog);
		std::cout << "Failed to link shader program - ERR: " << InfoLog << std::endl;
	}


	// Since the Program has created an executable with the shaders we can delete the originals
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}

void Shader::Activate() {
	glUseProgram(ID);
}

void Shader::Delete() {
	glDeleteProgram(ID);
}