#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

struct VertexData
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat textureCoord[2];
    GLfloat texInd[1];
};

bool loadAssImp(
	const aiMesh * mesh, 
	std::vector<unsigned int> & indices,
	std::vector<VertexData> & vertices,
    double scale
);

#endif
