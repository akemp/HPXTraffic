#include "headers.hpp"
#include "astar.hpp"

int main( void )
{
    int width = 512, height = width;

    MAP_WIDTH = width;
    MAP_HEIGHT = height;
    
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

	// Our problem defines the world as a 2d array representing a terrain
	// Each element contains an integer from 0 to 5 which indicates the cost 
	// of travel across the terrain. Zero means the least possible difficulty 
	// in travelling (think ice rink if you can skate) whilst 5 represents the 
	// most difficult. 9 indicates that we cannot pass.

	// Create an instance of the search class...


    for (int i = 0; i < MAP_WIDTH; ++i)
    {
        vector<int> tmap;
        for (int j = 0; j < MAP_HEIGHT; ++j)
        {
            tmap.push_back(zones[i][j]);
        }
        mapper.push_back(tmap);
    }

	AStarSearch<MapSearchNode> astarsearch;

	unsigned int SearchCount = 0;

	const unsigned int NumSearches = 1;
    vector<pair<double,double>> path;
	while(SearchCount < NumSearches)
	{

		// Create a start state
		MapSearchNode nodeStart;
		nodeStart.x = accessLocs[0][0];
		nodeStart.y = accessLocs[0][1]; 

		// Define the goal state
		MapSearchNode nodeEnd;
		nodeEnd.x = accessLocs[513][0];						
		nodeEnd.y = accessLocs[513][1]; 
		
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


				// Once you're done with the solution you can free the nodes up
				astarsearch.FreeSolutionNodes();

	
		}
		else if( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED ) 
		{
			cout << "Search terminated. Did not find goal state\n";
		
		}


		SearchCount ++;

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

    Mesh terrain(vertex_data,indices, LoadShaders( "vert.glsl", "fragTerra.glsl" ), loadDDS("dirt.dds"));//, createMap(roads,width,height));
    
    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();

    
    createSquares(zones,vertex_data,indices);

    Mesh roads(vertex_data,indices, LoadShaders( "vert.glsl", "fragTerra.glsl" ), loadDDS("asphalt.dds"));
    
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("car.obj", aiProcess_FlipUVs);
    
    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();
    
    loadAssImp(scene->mMeshes[0],indices, vertex_data, 0.008);

    

    terrain.move = vec3(0.0,-4.0,0.0);
    roads.move = vec3(0.0,-3.999,0.0);
    vector<Mesh> cars;
    Mesh car(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("delorean.dds"));

    car.rot = vec3(0,90,0);

    for (int i = 0; i < 1; ++i)
    {
        car.move = vec3(0.5, -4.0, 0.75);
        cars.push_back(car);
    }

	double lastTime = glfwGetTime();
	int nbFrames = 0;
    double last = glfwGetTime();
    double totalTime = 0;
    double timescale = 500.0;
	do{
        //void fps(int& nbFrames, double& totalTime, double& lastTime, double &last)
        double elapsed = fps(nbFrames, totalTime, lastTime, last);
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        terrain.draw();
        roads.draw();
        for (int i = 0; i < cars.size(); ++i)
        {
            int low = std::min<int>(floor(totalTime/timescale), path.size()-1);
            int high = std::min<int>(ceil(totalTime/timescale), path.size()-1);
            double diff = std::min<double>(totalTime/timescale-floor(totalTime/timescale), 1.0);
            vec2 moving = vec2(path[low].first*(1.0-diff)+path[high].first*diff,path[low].second*(1.0-diff)+path[high].second*diff);
            cars[i].move = vec3(0.5+moving[0], -4.0, 0.5+moving[1]);
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

