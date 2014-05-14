
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


using namespace glm;
using namespace std;

struct Mesh2d
{
    Mesh2d()
    {
    };
    Mesh2d(vector<VertexData> vert, vector<unsigned int> ind, GLuint pID, GLuint Tex)
    {

        programID = pID;
	    // Get a handle for our "myTextureSampler" uniform
	    TextureID  = glGetUniformLocation(programID, "tex");
        Texture = Tex;

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
        
		// Use our shader
		glUseProgram(programID);
        
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(TextureID, 0);

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
    ~Mesh2d()
    {
        //glDeleteBuffers(1, &vertices.second);
        //glDeleteBuffers(1, &indices.second);
	    // Cleanup VBO and shader
	    //glDeleteProgram(programID);
	    //glDeleteTextures(1, &Texture);
    }

    pair<vector<VertexData>,GLuint> vertices;
    pair<vector<unsigned int>,GLuint> indices;
    int num_indices;
    GLuint MatrixID, ModelMatrixID, ViewMatrixID, programID, TextureID, Texture;

};

struct Mesh
{
    Mesh()
    {
    };
    Mesh(vector<VertexData> vert, vector<unsigned int> ind, GLuint pID, GLuint Tex)
    {

        move = vec3(0,0,0);
        rot = vec3(0,0,0);


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

    pair<vector<VertexData>,GLuint> vertices;
    pair<vector<unsigned int>,GLuint> indices;
    int num_indices;
    vec3 move, rot;
    GLuint MatrixID, ModelMatrixID, ViewMatrixID, programID, TextureID, Texture;
    GLuint RoadID, RoadMap;

};


int startup(int W, int H, bool use2d = false)
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	if( !glfwOpenWindow( W, H, 0,0,0,0, 32,0, GLFW_WINDOW ) )
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	if (!use2d)
	{
		glfwDisable( GLFW_MOUSE_CURSOR );
		glfwSetMousePos(W/2, H/2);
	}


	glfwSetWindowTitle( "Traffic sim" );

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );

	// Dark blue background
	glClearColor(0.95f, 0.95f, 0.95f, 0.0f);


	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);       // Blending Function For Translucency Based On Source Alpha 
    glEnable(GL_BLEND);     // Turn Blending On
    glEnable(GL_ALPHA_TEST);
	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);
    

    glMatrixMode( GL_PROJECTION ); //Switch to setting the camera perspective

    //Set the camera perspective

    glLoadIdentity(); //reset the camera

    gluPerspective( 45.0f,                      //camera angle

                (GLfloat)W/(GLfloat)H, //The width to height ratio

                 0.001f,                          //The near z clipping coordinate

                110.0f );    
	// For speed computation

    return 0;
}

double fps(int& nbFrames, double& totalTime, double& lastTime, double &last)
{
	// Measure speed
	double currentTime = glfwGetTime();
	nbFrames++;
    double diff = currentTime-last;
    totalTime += diff;
        
	if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
		// printf and reset
		printf("%f ms/frame\n", 1000.0/double(nbFrames));
		nbFrames = 0;
		lastTime += 1.0;
	}
    return diff; 
}
