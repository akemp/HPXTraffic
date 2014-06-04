#ifndef GRAPHICS
#define GRAPHICS

#include <queue>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>


struct Mesh
{
    Mesh()
    {
    };
    Mesh(std::vector<VertexData> vert, std::vector<unsigned int> ind, GLuint pID, GLuint Tex)
    {

        using namespace glm;
        using namespace std;
        move = vec3(0,0,0);
        rot = vec3(0,0,0);

        if (vert.size() <= 0 || ind.size() <= 0)
        {
            exit(1);
        }

        programID = pID;
	    // Get a handle for our "myTextureSampler" uniform
	    TextureID  = glGetUniformLocation(programID, "tex");
        Texture = Tex;


	    // Get a handle for our "MVP" uniform
	    MatrixID = glGetUniformLocation(programID, "projection");
	    ViewMatrixID = glGetUniformLocation(programID, "modelview");
	    ModelMatrixID = glGetUniformLocation(programID, "model");

        glGenBuffers(1, &vertices.second);
        vertices.first = vert;
        glBindBuffer(GL_ARRAY_BUFFER, vertices.second);
        glBufferData(GL_ARRAY_BUFFER, vertices.first.size()*sizeof(VertexData), &vertices.first[0], GL_STATIC_DRAW);

        glGenBuffers(1, &indices.second);
	    indices.first = (ind);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.second);
        num_indices = indices.first.size();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*sizeof(unsigned int), &indices.first[0], GL_STATIC_DRAW);
    };

    void draw()
    {
        using namespace glm;
        using namespace std;
		// Use our shader
		glUseProgram(programID);
        
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(TextureID, 0);

        if (RoadMap != NULL)
        {
		    glActiveTexture(GL_TEXTURE1);
		    glBindTexture(GL_TEXTURE_2D, RoadMap);
		    glUniform1i(RoadID, 1);
        }

		// Compute the MVP matrix from keyboard and mouse input
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0), move) * glm::rotate(glm::mat4(1.0f), rot.x, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rot.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
	    
        //mov += 0.01;
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);


		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

        
        
        glBindBuffer(GL_ARRAY_BUFFER, vertices.second);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)(3*sizeof(GLfloat)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)(6*sizeof(GLfloat)));
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)(8*sizeof(GLfloat)));
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.second);
        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
    }
    glm::mat4 getModelMatrix()
    {
        glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0), move) * glm::rotate(glm::mat4(1.0f), rot.x, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rot.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
	    return ModelMatrix;
    }
    ~Mesh()
    {
        //glDeleteBuffers(1, &vertices.second);
        //glDeleteBuffers(1, &indices.second);
	    // Cleanup VBO and shader
	    //glDeleteProgram(programID);
	    //glDeleteTextures(1, &Texture);
    }

    std::pair<std::vector<VertexData>,GLuint> vertices;
    std::pair<std::vector<unsigned int>,GLuint> indices;
    int num_indices;
    glm::vec3 move, rot;
    GLuint MatrixID, ModelMatrixID, ViewMatrixID, programID, TextureID, Texture;
    GLuint RoadID, RoadMap;

};


int startup(int W, int H, bool use2d = false);

double fps(int& nbFrames, double& totalTime, double& lastTime, double &last);

#endif