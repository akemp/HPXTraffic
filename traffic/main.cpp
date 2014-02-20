#include "headers.hpp"

//#include "vehicles.hpp"
typedef adjacency_list < listS, vecS, directedS,
no_property, property < edge_weight_t, double > > graph_t;
typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef std::pair<int, int> Edge;

void outputPath(vector<int> nodes, graph_t g, char* name,  property_map<graph_t, edge_weight_t>::type weightmap, string path)
{
  ofstream dot_file(path + ".dot");

  dot_file << "digraph D {\n"
    << "  rankdir=LR\n"
    << "  size=\"4,3\"\n"
    << "  ratio=\"fill\"\n"
    << "  edge[style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";
  int indexer = 0;
  graph_traits < graph_t >::edge_iterator ei, ei_end;
  for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    graph_traits < graph_t >::edge_descriptor e = *ei;
    graph_traits < graph_t >::vertex_descriptor
      u = source(e, g), v = target(e, g);
    dot_file << name[u] << " -> " << name[v]
      << "[label=\"" << get(weightmap, e) << "\"";
      bool contains = false;
      if (indexer < nodes.size() &&  nodes[indexer] == v)
      {
        dot_file << ", color=\"black\"";
        indexer++;
      }
    else
      dot_file << ", color=\"grey\"";
    dot_file << "]";
  }
  dot_file << "}";
  return;
}

int runProgram()
{
    int width = 64, height = width;


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
    
    
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("car.obj", aiProcess_FlipUVs);
    
    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();
    
    loadAssImp(scene->mMeshes[0],indices, vertex_data, 0.008);

    

    terrain.move = vec3(0.0,-4.0,0.0);

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


		// Swap buffers
		glfwSwapBuffers();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
		   glfwGetWindowParam( GLFW_OPENED ) );


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

    return 0;
}

int outputPaths()
{   

    const int num_nodes = 5;
    enum nodes { A, B, C, D, E };
    char name[] = "ABCDE";
    Edge edge_array[] = { Edge(A, B), Edge(A, C), Edge(A, D), Edge(A, E),
         Edge(B, A), Edge(C, A), Edge(D, A), Edge(E, A)
    };


    double weights[] = { 5, 6, 2, 5.9,2.5, 3.5, 2, 5.9 };
    int num_arcs = sizeof(edge_array) / sizeof(Edge);

    graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);

    property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
    vector<vertex_descriptor> p(num_vertices(g));
    vector<double> d(num_vertices(g));
    vertex_descriptor s = vertex(B, g);

    dijkstra_shortest_paths(g, s,
                            predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g))).
                            distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g))));

    graph_traits < graph_t >::vertex_iterator vi, vend;
    vector<int> nodes;
    {
        int target = vertex(C, g);
        vector<double> distances;
            do{
                nodes.push_back(target);
                distances.push_back(d[target]);
                target = p[target];
            }while(target != s);
    }

    outputPath(nodes,  g, name,  weightmap, "out");
    
    return 0;
}

int main( void )
{
	outputPaths();
	return 0;
}

