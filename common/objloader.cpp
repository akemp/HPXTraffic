#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include <glm/glm.hpp>

#include "objloader.hpp"
#include <iostream>

using std::string;
using std::vector;
using std::find;
using std::cout;
using std::endl;

bool loadAssImp(
	const aiMesh * mesh, 
	std::vector<unsigned int> & indices,
	std::vector<VertexData> & vertices,
    double scale
)
{
    vertices.resize(mesh->mNumVertices);
	// Fill vertices positions
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D pos = mesh->mVertices[i];
        for (int j = 0; j < 3; ++j)
            vertices[i].position[j] = pos[j]*scale;
        vertices[i].texInd[0] = 0.0;
	}
	// Fill vertices texture coordinates
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
		
		aiVector3D pos = mesh->mVertices[i];
        for (int j = 0; j < 2; ++j)
            vertices[i].textureCoord[j] = UVW[j];
	}
	// Fill vertices normals
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
//		aiVector3D n = mesh->mNormals[i];
        for (int j = 0; j < 3; ++j)
            vertices[i].normal[j] = mesh->mNormals[i][j];
	}

	// Fill face indices
	indices.reserve(3*mesh->mNumFaces);
	for (unsigned int i=0; i<mesh->mNumFaces; i++){
		// Assume the model has only triangles.
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}
	// The "scene" pointer will be deleted automatically by "importer"
    return true;
}
