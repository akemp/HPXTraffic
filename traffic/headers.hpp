#ifndef HEADERS
#define HEADERS

//#include <hpx/hpx_init.hpp>

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <queue>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <common/nanoflann.hpp>

using namespace boost; 
using namespace nanoflann;


#define WIDTH 1024
#define HEIGHT 600


using namespace std;



struct PointCloud
{

	std::vector<vec2>  pts;

	// Must return the number of data points
	inline size_t kdtree_get_point_count() const { return pts.size(); }

	// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
	inline double kdtree_distance(const double *p1, const size_t idx_p2,size_t size) const
	{
        const double d0=p1[0]-pts[idx_p2][0];
		const double d1=p1[1]-pts[idx_p2][1];
		return d0*d0+d1*d1;
	}

	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline double kdtree_get_pt(const size_t idx, int dim) const
	{
		if (dim==0) return pts[idx][0];
		else return pts[idx][1];
	}

	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX &bb) const { return false; }

};

// construct a kd-tree index:
typedef KDTreeSingleIndexAdaptor<
	L2_Simple_Adaptor<double, PointCloud >,
	PointCloud,
	2 /* dim */
	> kd_tree;
    

struct Road
{
    bool left, right, up, down;
    int x,y;
    Road(int i, int j)
    {
        x = i;
        y = j;
        left = false;
        right = false;
        up = false;
        down = false;
    }
};


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
    Mesh(vector<VertexData> vert, vector<unsigned int> ind, GLuint pID, GLuint Tex, GLuint RoadM = NULL)
    {

        move = vec3(0,0,0);
        rot = vec3(0,0,0);
        postMove = vec3(0,0,0);


        programID = pID;
	    // Get a handle for our "myTextureSampler" uniform
	    TextureID  = glGetUniformLocation(programID, "tex");
        Texture = Tex;
        if (RoadM != NULL)
        {
	        RoadID  = glGetUniformLocation(programID, "roadMap");
        }
        RoadMap = RoadM;


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
        glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0), move) * glm::rotate(glm::mat4(1.0f), rot.x, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rot.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rot.z, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0), postMove);
	    
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
        glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0), move) * glm::rotate(glm::mat4(1.0f), rot.x, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rot.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), rot.z, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::mat4(1.0), postMove);
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
    vec3 move, rot, postMove;
    GLuint MatrixID, ModelMatrixID, ViewMatrixID, programID, TextureID, Texture;
    GLuint RoadID, RoadMap;

};



GLuint createMap(vector<Road> roads, int width, int height){


	// Some BMP files are misformatted, guess missing information
	int imageSize=width*height*4; // : one byte for each Red, Green and Blue component

	// Create a buffer
	unsigned char* data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
    for (int i = 0; i < imageSize; ++i)
    {
        data[i] = 100;
    }


	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete [] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}


int startup(int W = WIDTH, int H = HEIGHT, bool use2d = false)
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
		glfwSetMousePos(WIDTH/2, HEIGHT/2);
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

                (GLfloat)WIDTH/(GLfloat)HEIGHT, //The width to height ratio

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


void createRect(vector<VertexData>& vertex_data,vector<unsigned int>& indices, vec2 i, vec2 j, double height)
{
    int index = 0;
    VertexData data;
    data.texInd[0] = -1;
    data.position[0] = i[0];
    data.position[1] = height;
    data.position[2] = j[0];
    data.normal[0] = 0;
    data.normal[1] = 1;
    data.normal[2] = 0;
    data.textureCoord[0] = 0;
    data.textureCoord[1] = 0;
    vertex_data.push_back(data);


    data.position[0] = i[1];
    data.position[1] = height;
    data.position[2] = j[0];
    data.normal[0] = 0;
    data.normal[1] = 1;
    data.normal[2] = 0;
    data.textureCoord[0] = 1;
    data.textureCoord[1] = 0;
    vertex_data.push_back(data);


    data.position[0] = i[0];
    data.position[1] = height;
    data.position[2] = j[1];
    data.normal[0] = 0;
    data.normal[1] = 1;
    data.normal[2] = 0;
    data.textureCoord[0] = 0;
    data.textureCoord[1] = 1;
    vertex_data.push_back(data);


    data.position[0] = i[1];
    data.position[1] = height;
    data.position[2] = j[1];
    data.normal[0] = 0;
    data.normal[1] = 1;
    data.normal[2] = 0;
    data.textureCoord[0] = 1;
    data.textureCoord[1] = 1;
    vertex_data.push_back(data);


    indices.push_back(index);
    indices.push_back(index+2);
    indices.push_back(index+1);

    indices.push_back(index+3);
    indices.push_back(index+1);
    indices.push_back(index+2);

}

void createSquare(double i, double j, vector<VertexData>& vertex_data, vector<unsigned int>& indices, int &index,
	vec4 height = vec4(0,0,0,0), double scale = 1, double texInd = -1)
{
    VertexData data;
    data.texInd[0] = texInd;
    data.position[0] = i;
    data.position[1] = height[0];
    data.position[2] = j;
    data.normal[0] = 0;
    data.normal[1] = 1;
    data.normal[2] = 0;
    data.textureCoord[0] = 0;
    data.textureCoord[1] = 0;
    vertex_data.push_back(data);


    data.position[0] = i+scale;
    data.position[1] = height[1];
    data.position[2] = j;
    data.normal[0] = 0;
    data.normal[1] = 1;
    data.normal[2] = 0;
    data.textureCoord[0] = 1;
    data.textureCoord[1] = 0;
    vertex_data.push_back(data);


    data.position[0] = i;
    data.position[1] = height[2];
    data.position[2] = j+scale;
    data.normal[0] = 0;
    data.normal[1] = 1;
    data.normal[2] = 0;
    data.textureCoord[0] = 0;
    data.textureCoord[1] = 1;
    vertex_data.push_back(data);


    data.position[0] = i+scale;
    data.position[1] = height[3];
    data.position[2] = j+scale;
    data.normal[0] = 0;
    data.normal[1] = 1;
    data.normal[2] = 0;
    data.textureCoord[0] = 1;
    data.textureCoord[1] = 1;
    vertex_data.push_back(data);


    indices.push_back(index);
    indices.push_back(index+2);
    indices.push_back(index+1);

    indices.push_back(index+3);
    indices.push_back(index+1);
    indices.push_back(index+2);

    index += 4;
}

void createTerrain(int maxx, int maxy, vector<VertexData>& vertex_data, vector<unsigned int>& indices, double scale = 1.0, double heights = 1.0)
{
    int index = 0;
    
	//CImg <unsigned char> image("heightmap.ppm"); 
    //image.resize(maxx+1,maxy+1);
    for (int i = 0; i < maxx; ++i)
    {
        for (int j = 0; j < maxy; ++j)
        {
            //vec4 height = vec4(image(i,j),image(i+1,j),image(i,j+1),image(i+1,j+1));
             // height /= -heights;
            createSquare(i*scale,j*scale,vertex_data,indices,index);//,height,scale);
        }
    }
}

vector<Road> createRoads(vector<vector<int>> zones)
{
    vector<Road> roads;
    for (int i = 0; i < zones.size(); i ++)
    {
        for (int j = 0; j < zones[i].size(); j++)
        {
            if (zones[i][j] != 9)
            {
                Road r(i,j);
                if (i > 0)
                    if (zones[i-1][j] != 9)
                        r.down = true;
                if (i < zones.size() - 1)
                    if (zones[i+1][j] != 9)
                        r.up = true;
                if (j > 0)
                    if (zones[i][j-1] != 9)
                        r.left = true;
                if (j < zones[0].size() - 1)
                    if (zones[i][j+1] != 9)
                        r.right = true;
                roads.push_back(r);
            }
        }
    }
    return roads;
}

void createSquares(vector<vector<int>> zones, vector<VertexData>& vertex_data, vector<unsigned int>& indices)
{
    vector<Road> roads = createRoads(zones);
    int index = 0;
    for (int i = 0; i < roads.size(); ++i)
    {
        Road r = roads[i];
        createSquare(r.x,r.y,vertex_data,indices,index);
    }
}


#endif