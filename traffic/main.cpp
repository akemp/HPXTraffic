#include "headers.hpp"
#include "astar.hpp";

int main( void )
{
    int code = startup();
    if (code != 0)
        return code;

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Load the texture
	


	vector<unsigned int> indices;
    vector<VertexData> vertex_data;
    
    int width = 512, height = width;

    createTerrain(width,height,vertex_data,indices,0.1, 3000);
    
    vector<vector<int>> zones(width, vector<int>(height, 0));

    for (int i = 0; i < width; ++i)
    {
        for (int j = 0; j < height; ++j)
        {
            if (i % 20 == 0 || j % 8 == 0)
                zones[i][j] = 1;
        }
    }

    vector<Road> roads = createRoads(zones);

    Mesh terrain(vertex_data,indices, LoadShaders( "vert.glsl", "fragTerra.glsl" ), loadDDS("dirt.dds"), createMap(roads,width,height));
    
    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();

    
    //createSquares(zones,vertex_data,indices);

    //Mesh roads(vertex_data,indices, LoadShaders( "vert.glsl", "fragTerra.glsl" ), loadDDS("asphalt.dds"));
    
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("car.obj", aiProcess_FlipUVs);
    
    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();
    
    loadAssImp(scene->mMeshes[0],indices, vertex_data, 0.008);

    

    terrain.move = vec3(0.0,-4.0,0.0);
    //roads.move = vec3(0.0,-3.9,0.0);
    vector<Mesh> cars;
    Mesh car(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("delorean.dds"));

    car.rot = vec3(0,90,0);

    for (int i = 0; i < 1; ++i)
    {
        car.move = vec3(i+0.5, -3.9, 0.75);
        cars.push_back(car);
    }

	double lastTime = glfwGetTime();
	int nbFrames = 0;
    double last = glfwGetTime();
    double totalTime = 0;
	do{
        //void fps(int& nbFrames, double& totalTime, double& lastTime, double &last)
        double elapsed = fps(nbFrames, totalTime, lastTime, last);
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        terrain.draw();
        //roads.draw();
        for (int i = 0; i < cars.size(); ++i)
        {
            cars[i].draw();
        }

		// Swap buffers
		glfwSwapBuffers();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
		   glfwGetWindowParam( GLFW_OPENED ) );


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

    return 0;
}

