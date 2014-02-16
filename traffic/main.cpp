#include "headers.hpp"
#include "astar.hpp"
#include "vehicles.hpp"

int runProgram()
{
    int width = 64, height = width;
	// Our problem defines the world as a 2d array representing a terrain
	// Each element contains an integer from 0 to 5 which indicates the cost 
	// of travel across the terrain. Zero means the least possible difficulty 
	// in travelling (think ice rink if you can skate) whilst 5 represents the 
	// most difficult. 9 indicates that we cannot pass.
    
    vector<vector<int>> zones(width, vector<int>(height, 9));
    
    vector<ivec2> accessLocs;
    for (int i = 0; i < width; ++i)
    {
        for (int j = 0; j < height; ++j)
        {
            if (i % 20 == 0 || j % 8 == 0)
            {
                zones[i][j] = 1;
                accessLocs.push_back(ivec2(i,j));
            }
        }
    }


	// Create an instance of the search class...


    for (int i = 0; i < width; ++i)
    {
        vector<int> tmap;
        for (int j = 0; j < height; ++j)
        {
            tmap.push_back(zones[i][j]);
        }
        mapper.push_back(tmap);
    }

	AStarSearch<MapSearchNode> astarsearch;
    
    vector<vector<pair<double,double>>> paths;
    for (int i = 0; i < 100; ++i)
	{
        
        vector<pair<double,double>> path;
		// Create a start state
		MapSearchNode nodeStart;
        int start = rand()%(accessLocs.size());
		nodeStart.x = accessLocs[start][0];
		nodeStart.y = accessLocs[start][1]; 

		// Define the goal state
		MapSearchNode nodeEnd;
        int dest = rand()%(accessLocs.size());
		nodeEnd.x = accessLocs[dest][0];						
		nodeEnd.y = accessLocs[dest][1]; 
		
		// Set Start and goal states
		
		astarsearch.SetStartAndGoalStates( nodeStart, nodeEnd );

		unsigned int SearchState;
		unsigned int SearchSteps = 0;

		do
		{
			SearchState = astarsearch.SearchStep();

			SearchSteps++;


		}
		while( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SEARCHING );

		if( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SUCCEEDED )
		{

				MapSearchNode *node = astarsearch.GetSolutionStart();

				int steps = 0;

                path.push_back(node->GetNodeInfo());
				for( ;; )
				{
					node = astarsearch.GetSolutionNext();

					if( !node )
					{
						break;
					}

                    path.push_back(node->GetNodeInfo());
					steps ++;
				
				};

                paths.push_back(path);
				// Once you're done with the solution you can free the nodes up
				astarsearch.FreeSolutionNodes();

	
		}
		else if( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED ) 
		{
			cout << "Search terminated. Did not find goal state\n";
		
		}

		astarsearch.EnsureMemoryFreed();
	}

    int code = startup();
    if (code != 0)
        return code;

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Load the texture
	


	vector<unsigned int> indices;
    vector<VertexData> vertex_data;
    

    createTerrain(width,height,vertex_data,indices,1.0, 300);
    
    //vector<Road> road = createRoads(zones);

    Mesh terrain(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("dirt.dds"));//, createMap(roads,width,height));
    
    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();

    
    createSquares(zones,vertex_data,indices);

    Mesh roads(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("asphalt.dds"));
    
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("car.obj", aiProcess_FlipUVs);
    
    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();
    
    loadAssImp(scene->mMeshes[0],indices, vertex_data, 0.008);

    

    terrain.move = vec3(0.0,-4.0,0.0);
    roads.move = vec3(0.0,-3.999,0.0);
    vector<Car> cars;
    Mesh car(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("delorean.dds"));



    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();


    createRect(vertex_data,indices,vec2(0.175,-0.175),vec2(0.4,-0.4),0.01);

    Mesh shadow(vertex_data,indices, LoadShaders( "vert.glsl", "fragTerra.glsl" ), loadDDS("shadow.dds"));

    
    //car.move = vec3(0.5, -4.0, 0.75);
    double timescale = 200.0;
    vec3 postMove = vec3(-0.25,0, 0.25);
    for (int i = 0; i < paths.size(); ++i)
    {
        cars.push_back(Car(car,shadow,timescale,paths[i], i, postMove));
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
		computeMatricesFromInputs();
        
        terrain.draw();
        roads.draw();

        
	    PointCloud cloud;

	    // Generate points:
        for (int i = 0; i < cars.size(); ++i)
        {
            cloud.pts.push_back(vec2(cars[i].mesh.move[0],cars[i].mesh.move[2]));
        }


	    kd_tree   index(2 /*dim*/, cloud, KDTreeSingleIndexAdaptorParams(10 /* max leaf */) );
	    index.buildIndex();

        
        for (int i = 0; i < cars.size(); ++i)
        {
            cars[i].draw(elapsed,index, cars);
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

int debugTiles()
{

    int code = startup(800,800,true);
    if (code != 0)
        return code;
	
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	vector<VertexData> vert;
	vector<unsigned int> indices;
	int count = 0;
	double scale = 1.0/64.0;
	for (int i = 0; i < 64; ++i)
	{
		for (int j = 0; j < 64; ++j)
		{
			createSquare(i*scale,j*scale,vert,indices,count,vec4(0,0,0,0),scale,1.0);//,height,scale);
		}
	}
	
	Mesh2d m(vert, indices, LoadShaders( "vert2d.glsl", "frag2d.glsl" ), loadDDS("asphalt.dds"));

	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m.draw();

		glfwSwapBuffers();
        
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
		   glfwGetWindowParam( GLFW_OPENED ) );


	// Close OpenGL window and terminate GLFW
	glfwTerminate();


	return 0;
}

int main( void )
{
	runProgram();
	return 0;
}

