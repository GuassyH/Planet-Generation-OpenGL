#include "SphereSide.h"
#include <glm/gtc/type_ptr.hpp> // For glm::value_ptr


SphereSide::SphereSide(unsigned int& resolution, float& radius, glm::mat4& rotationMatrix, float& tile) {

    // unsigned int numVerts = (resolution + 1) * (resolution + 1);
    // std::vector <Vertex> verts = std::vector<Vertex>(numVerts);
    // std::vector <Vertex> verts;
    std::vector <GLuint> inds;

 


    // runComputeShader(resolution, radius, tile, rotationMatrix, numVerts, verts);

    
    for (unsigned int i = 0; i < (resolution * resolution) + resolution; i++) {
        // Bottom tri
        if ((i + 1) % (resolution + 1) != 0.0f) {
            inds.push_back(i);
            inds.push_back(i + 1);
            inds.push_back(i + resolution + 1);
        }
        if (i % (resolution + 1) != 0.0f) {
            // Top tri
            inds.push_back(i);
            inds.push_back(i + resolution + 1);
            inds.push_back(i + resolution);
        }
    }
    
    SphereSide::indices = inds;
}

