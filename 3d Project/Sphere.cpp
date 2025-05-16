#include "Sphere.h"
#include "SphereSide.h"
#include <algorithm> // for std::copy



void Sphere::runComputeSphere(unsigned int& resolution, float& radius, float& tile, unsigned int& numVerts, std::vector<Vertex>& verts) {
    int success;
    char InfoLog[512];

    Sphere::computeVerts = std::vector<ComputeVertex>(numVerts);


    GLuint workGroupSize = 1;
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


    std::string computeShaderString = layoutStr + "//" + get_file_contents("ComputeSphere.comp");
    const char* computeShaderSource = computeShaderString.c_str();

    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderSource, NULL);
    glCompileShader(computeShader);
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(computeShader, 512, 0, InfoLog);
        std::cout << "Failed to compile compute shader - ERR: " << InfoLog << std::endl;
    }

    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(computeProgram, 512, 0, InfoLog);
        std::cout << "Failed to link compute program - ERR: " << InfoLog << std::endl;
    }

    glUseProgram(computeProgram);

    // assign buffer
    GLuint vertBuff;
    GLsizeiptr vertBuffSize = sizeof(ComputeVertex) * numVerts;
    glGenBuffers(1, &vertBuff);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertBuff);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertBuffSize, computeVerts.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertBuff); // THIS IS ESSENTIAL

    glUniform1i(glGetUniformLocation(computeProgram, "resolution"), resolution);
    glUniform1i(glGetUniformLocation(computeProgram, "numVerts"), numVerts);
    glUniform1f(glGetUniformLocation(computeProgram, "radius"), radius);
    glUniform1f(glGetUniformLocation(computeProgram, "tile"), tile);


    
    // Create 6 SphereSide instances (like the 6 sides of a cube sphere)
    for (unsigned int i = 0; i < 6; ++i) {
        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        if (i < 4) {
            rotationMatrix = glm::rotate(glm::radians(i * 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (i == 4) {
            rotationMatrix = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }
        else if (i == 5) {
            rotationMatrix = glm::rotate(glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }

        std::string thisRotStr = "rotationMat" + std::to_string(i + 1);
        const char* thisRotMat = thisRotStr.c_str();
        glUniformMatrix4fv(glGetUniformLocation(computeProgram, thisRotMat), 1, GL_FALSE, glm::value_ptr(rotationMatrix));
    }
    





    glDispatchCompute((resolution + 1) / layoutGroupX, (resolution + 1) / layoutGroupY, layoutGroupZ);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertBuff);

    // Map buffer for reading
    ComputeVertex* ptr = (ComputeVertex*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, vertBuffSize, GL_MAP_READ_BIT);
    if (ptr == nullptr) {
        std::cerr << "Failed to map buffer for reading!" << std::endl;
    }
    else {
        for (unsigned int i = 0; i < numVerts; ++i) {
            computeVerts[i] = ptr[i]; // copy from GPU buffer to CPU vector

            verts[i].position = glm::vec3(computeVerts[i].position);
            verts[i].normal = glm::vec3(computeVerts[i].normal);
            verts[i].color = glm::vec3(computeVerts[i].color);
            verts[i].texUV = glm::vec2(computeVerts[i].texUV);
        }
    }


    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER); // VERY IMPORTANT

    glDeleteShader(computeShader);
    glDeleteProgram(computeProgram);

    glDeleteBuffers(1, &vertBuff);
}




Sphere::Sphere(unsigned int resolution, float radius, float tile) {

    std::vector<SphereSide> sphereSides;
    Sphere::radius = radius;
    Sphere::resolution = resolution;
    Sphere::tile = tile;

    
    glm::mat4 rotationMatrix = glm::mat4(1.0f);

    
    // Create 6 SphereSide instances (like the 6 sides of a cube sphere)
    for (unsigned int i = 0; i < 6; ++i) {
        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        if (i < 4) {
            sphereSides.emplace_back(resolution, radius, rotationMatrix, tile);
        }
        else if (i == 4) {
            sphereSides.emplace_back(resolution, radius, rotationMatrix, tile);
        }
        else if (i == 5) {
            sphereSides.emplace_back(resolution, radius, rotationMatrix, tile);
        }

    }

    
    
    unsigned int numVerts = ((resolution + 1) * (resolution + 1)) * 6;
    std::vector<Vertex> verts(numVerts);

    runComputeSphere(resolution, radius, tile, numVerts, verts);
    vertices = verts;




    GLuint vertexOffset = 0;
    for (const auto& side : sphereSides) {
        // Append vertices

        for (GLuint index : side.indices) {
            indices.push_back(index + vertexOffset);
        }

        vertexOffset += ((resolution + 1) * (resolution + 1));
    }


    // Free  allocated memory from the data that the sides created since all that data is now in the Sphere class
    for (unsigned int i = 0; i < 6; ++i) {
        std::vector<unsigned int>().swap(sphereSides[i].indices);
    }

    std::vector<SphereSide>().swap(sphereSides);
}



Sphere::~Sphere() {
    std::vector<Vertex>().swap(vertices);
    std::vector<GLuint>().swap(indices);
}