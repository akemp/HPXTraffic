#include "headers.hpp"

int main()
{
    using namespace std;
    using namespace boost;
    using namespace glm;
    float len = 200;
	LineMesh roadsegs = generateRoads(8,4, len);
    
    vector<Edge> egs;
    vector<vec2> inputted;
    

    vector<unsigned int> indices;
    vector<VertexData> vertex_data;

    vector<road> roads;

    generateRoadModels(roadsegs, indices, vertex_data,roads);
    
    vector<Edge> edge_vector;
    vector<double> weight_array;


    
    for (int i = 0; i < roads.size(); ++i)
    {
        for (int j = 0; j < roads[i].links.size(); ++j)
        {
            edge_vector.push_back(Edge(i, roads[i].links[j]->index));
            weight_array.push_back(glm::distance(roads[i].v0, roads[i].v1));
        }
    }
    
    graph_t g(edge_vector.begin(), edge_vector.end(), weight_array.begin(), roads.size());
    property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
    vector<vertex_descriptor> p(num_vertices(g));
    vector<double> d(num_vertices(g));
    pred_map pd = predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g))).
                            distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g)));

    vector<road*> roadsp;
    for (int i = 0; i < roads.size(); ++i)
    {
        roadsp.push_back(&roads[i]);
    }
    
    float scaler = 0.0105;

    for (int i = 0; i < roads.size(); ++i)
    {        
        roads[i].v0 += roads[i].dir*0.4f;
        roads[i].v1 -= roads[i].dir*0.4f;
    }

    
    int code = startup(1200,600);

    if (code != 0)
        return code;
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);


    Mesh terrain(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("asphalt.dds"));
    terrain.move.y = -3.0;

    
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    double last = glfwGetTime();
    float speed = 1.0;
    double totalTime = 0;
        int ncars = 1;
        
    vector<vehicle> vehicles;
    vector<Mesh> cars;

    indices = vector<unsigned int>();
    vertex_data = vector<VertexData>();
    
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("car.obj", aiProcess_FlipUVs);
    loadAssImp(scene->mMeshes[0],indices, vertex_data, 0.008*scaler/0.0225);
    
    Mesh car(vertex_data,indices, LoadShaders( "vert.glsl", "frag.glsl" ), loadDDS("delorean.dds"));
    car.move.y = -3.0;
    vehicles.reserve(ncars);
    cars.resize(ncars, car);
    float carsize = 0.45f;
    int counter = 0;
    for (int i = 0; i < ncars; ++i)
    {
        //vector<int> path = generatePath(i/10,0,g,weightmap,p,d,streetsp,pd);
        int start = counter%roads.size();
        int end = 0;//(i*321093)%streets.size();
        int index = start;
        int inc = 0;
        while (roads[index].cars.size() * carsize+carsize > roads[index].totaltime())
        {
            start = counter%roads.size();
            end = 0;//(i*321093+counter*12055)%streets.size();
            index = start;
            ++counter;
            ++inc;
            if (inc > 100)
                exit(1);
        }
        {
            vehicle pather(i, roadsp[index]);
            pather.destination = end;
            pather.progress = roads[index].cars.size() * carsize+i*0.0001f;
            pather.start = roads[index].v0;
            pather.dir = roads[index].dir;
            pather.dist = roads[index].totaltime();
            //pather.path = path;
            //pather.turn = Edge(path.front(), path[1]);
            vehicles.push_back(pather);
            roads[index].addvehicle(&vehicles[vehicles.size()-1]);
        }
    }
    do{	
        double elapsed = fps(nbFrames, totalTime, lastTime, last)*speed;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        computeMatricesFromInputs();

        terrain.draw();
        //elapsed = 1.0f;//glm::min(10.0,elapsed);
        //elapsed = 100.0f;
        float iters = 10.0f;
        for (float i = 0; i < iters; ++i)
            processCars(cars, vehicles, roads, 1.0f, 0.01f,g,weightmap,p,d,roadsp,pd, carsize,false);
        for (int i = 0; i < cars.size(); ++i)
            cars[i].draw();

        glfwSwapBuffers();

    } while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
                glfwGetWindowParam( GLFW_OPENED ) );
     glfwTerminate();
     vehicles.resize(0);

    return 0;
}