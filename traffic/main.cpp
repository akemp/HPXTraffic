#include "headers.hpp"

typedef adjacency_list < listS, vecS, directedS,
no_property, property < edge_weight_t, double > > graph_t;
typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef std::pair<int, int> Edge;

void outputPath(vector<int> nodes, graph_t g, vector<string> name,  property_map<graph_t, edge_weight_t>::type weightmap, string path)
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

    vector<string> name;
    name.push_back("A");
    name.push_back("B");
    name.push_back("C");
    name.push_back("D");
    name.push_back("E");
    vector<Edge> edge_vector;
    edge_vector.push_back(Edge(0, 1));
    edge_vector.push_back(Edge(0, 2));
    edge_vector.push_back(Edge(0, 3));
    edge_vector.push_back(Edge(0, 4));
    edge_vector.push_back(Edge(1, 0));
    edge_vector.push_back(Edge(2, 0));
    edge_vector.push_back(Edge(3, 0));
    edge_vector.push_back(Edge(4, 0));

    vector<double> weight_array;
    for (int i = 0; i < 8; ++i)
    {
        weight_array.push_back(i+1);
    }

    graph_t g(edge_vector.begin(), edge_vector.end(), weight_array.begin(), name.size());

    property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
    vector<vertex_descriptor> p(num_vertices(g));
    vector<double> d(num_vertices(g));
    vertex_descriptor s = vertex(1, g);

    dijkstra_shortest_paths(g, s,
                            predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g))).
                            distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g))));

    graph_traits < graph_t >::vertex_iterator vi, vend;
    vector<int> nodes;
    {
        int target = vertex(2, g);
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

